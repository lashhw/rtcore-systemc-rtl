#ifndef DEFAULT_SYSTEMC_BVH_HPP
#define DEFAULT_SYSTEMC_BVH_HPP

#include <numeric>
#include <algorithm>
#include <stack>
#include "triangle.hpp"
#include "bounding_box.hpp"

struct Bvh {
    struct Node {
        bool is_leaf() const { return num_trigs > 0; }

        BoundingBox bbox;
        int num_trigs;  // 0 when node != leaf
        union {
            int left_node_idx;  // used when node != leaf
            int first_trig_idx;  // used when node == leaf
        };
    };

    Bvh(const std::vector<Triangle> &unsorted_triangles);

    static const int BVH_MAX_DEPTH = 30;

    int num_triangles;
    Triangle *triangles;
    int num_nodes;
    Node *nodes;
};

// construct BVH
Bvh::Bvh(const std::vector<Triangle> &unsorted_triangles) : num_triangles(unsorted_triangles.size()) {
    // allocate temporary memory for BVH construction
    auto bboxes = std::make_unique<BoundingBox[]>(num_triangles);
    auto centers = std::make_unique<Vec3[]>(num_triangles);
    auto costs = std::make_unique<float[]>(num_triangles);
    auto marks = std::make_unique<bool[]>(num_triangles);
    auto sorted_references_data = std::make_unique<int[]>(3 * num_triangles);
    int *sorted_references[3] = { sorted_references_data.get(),
                                  sorted_references_data.get() + num_triangles,
                                  sorted_references_data.get() + 2 * num_triangles };
    auto tmp_nodes = std::make_unique<Node[]>(2 * num_triangles);
    triangles = new Triangle[num_triangles];

    // initially, there is only one node
    num_nodes = 1;
    int max_depth = 0;

    // initialize bboxes, centers, and tmp_nodes[0].bbox
    tmp_nodes[0].bbox.reset();
    for (int i = 0; i < num_triangles; i++) {
        bboxes[i] = unsorted_triangles[i].bounding_box();
        tmp_nodes[0].bbox.extend(bboxes[i]);
        centers[i] = unsorted_triangles[i].center();
    }

    std::cout << "Global bounding box: ";
    std::cout << "(" << tmp_nodes[0].bbox.bounds[0] << ", "
              << tmp_nodes[0].bbox.bounds[2] << ", "
              << tmp_nodes[0].bbox.bounds[4] << ") ";
    std::cout << "(" << tmp_nodes[0].bbox.bounds[1] << ", "
              << tmp_nodes[0].bbox.bounds[3] << ", "
              << tmp_nodes[0].bbox.bounds[5] << ") " << std::endl;

    // sort on x-coordinate
    std::iota(sorted_references[0], sorted_references[0] + num_triangles, 0);
    std::sort(sorted_references[0], sorted_references[0] + num_triangles,
              [&](int i, int j) { return centers[i].x < centers[j].x; });

    // sort on y-coordinate
    std::iota(sorted_references[1], sorted_references[1] + num_triangles, 0);
    std::sort(sorted_references[1], sorted_references[1] + num_triangles,
              [&](int i, int j) { return centers[i].y < centers[j].y; });

    // sort on z-coordinate
    std::iota(sorted_references[2], sorted_references[2] + num_triangles, 0);
    std::sort(sorted_references[2], sorted_references[2] + num_triangles,
              [&](int i, int j) { return centers[i].z < centers[j].z; });

    // initialize stack for BVH construction
    std::stack<std::array<int, 4>> stack;  // node_idx, begin, end, depth
    int node_idx = 0;
    int begin = 0;
    int end = num_triangles;
    int depth = 0;
    auto check_and_update_and_pop_stack = [&]() -> bool {
        if (stack.empty()) return false;
        node_idx = stack.top()[0];
        begin = stack.top()[1];
        end = stack.top()[2];
        depth = stack.top()[3];
        stack.pop();
        return true;
    };

    // recursion step for BVH construction (implemented by stack)
    while (true) {
        Node &curr_node = tmp_nodes[node_idx];
        int curr_num_primitives = end - begin;

        // this node should be a leaf node
        if (curr_num_primitives <= 1 || depth >= BVH_MAX_DEPTH) {
            curr_node.num_trigs = curr_num_primitives;
            curr_node.first_trig_idx = begin;
            if (check_and_update_and_pop_stack()) continue;
            else break;
        }

        float best_cost = FLT_MAX;
        int best_axis = -1;
        int best_split_index = -1;

        // find best split axis and split index
        for (int axis = 0; axis < 3; axis++) {
            BoundingBox tmp_bbox = BoundingBox::Empty();
            for (int i = end - 1; i > begin; i--) {
                tmp_bbox.extend(bboxes[sorted_references[axis][i]]);
                costs[i] = tmp_bbox.half_area() * (end - i);
            }

            tmp_bbox.reset();
            for (int i = begin; i < end - 1; i++) {
                tmp_bbox.extend(bboxes[sorted_references[axis][i]]);
                float cost = tmp_bbox.half_area() * (i + 1 - begin) + costs[i + 1];
                if (cost < best_cost) {
                    best_cost = cost;
                    best_axis = axis;
                    best_split_index = i + 1;
                }
            }
        }

        // if best_cost >= max_split_cost, this node should be a leaf node
        float max_split_cost = curr_node.bbox.half_area() * (curr_num_primitives - 1);
        if (best_cost >= max_split_cost) {
            curr_node.num_trigs = curr_num_primitives;
            curr_node.first_trig_idx = begin;
            if (check_and_update_and_pop_stack()) continue;
            else break;
        }

        // set bbox of left and right nodes
        int left_node_index = num_nodes;
        int right_node_index = num_nodes + 1;
        Node &left_node = tmp_nodes[left_node_index];
        Node &right_node = tmp_nodes[right_node_index];
        left_node.bbox.reset();
        right_node.bbox.reset();
        for (int i = begin; i < best_split_index; i++) {
            left_node.bbox.extend(bboxes[sorted_references[best_axis][i]]);
            marks[sorted_references[best_axis][i]] = true;
        }
        for (int i = best_split_index; i < end; i++) {
            right_node.bbox.extend(bboxes[sorted_references[best_axis][i]]);
            marks[sorted_references[best_axis][i]] = false;
        }

        // partition sorted_references of other axes and ensure their relative order
        int other_axis[2] = { (best_axis + 1) % 3, (best_axis + 2) % 3 };
        std::stable_partition(sorted_references[other_axis[0]] + begin,
                              sorted_references[other_axis[0]] + end,
                              [&](int i) { return marks[i]; });
        std::stable_partition(sorted_references[other_axis[1]] + begin,
                              sorted_references[other_axis[1]] + end,
                              [&](int i) { return marks[i]; });

        // now we are sure that this node is an internal node
        num_nodes += 2;
        curr_node.num_trigs = 0;
        curr_node.left_node_idx = left_node_index;
        max_depth = std::max(max_depth, depth + 1);

        int left_size = best_split_index - begin;
        int right_size = end - best_split_index;

        // process smaller subtree first
        if (left_size < right_size) {
            stack.push( { right_node_index, best_split_index, end, depth + 1 } );
            node_idx = left_node_index;
            begin = begin;
            end = best_split_index;
            depth = depth + 1;
        } else {
            stack.push( { left_node_index, begin, best_split_index, depth + 1 } );
            node_idx = right_node_index;
            begin = best_split_index;
            end = end;
            depth = depth + 1;
        }
    }

    std::cout << "BVH has " << num_nodes << " nodes and " << num_triangles
              << " triangles, with max_depth = " << max_depth << std::endl;

    // rearrange primitives based on sorted_references
    for (int i = 0; i < num_triangles; i++) triangles[i] = unsorted_triangles[sorted_references[0][i]];

    // copy nodes to device
    nodes = new Node[num_nodes];
    std::copy(tmp_nodes.get(), tmp_nodes.get() + num_nodes, nodes);
}

#endif //DEFAULT_SYSTEMC_BVH_HPP
