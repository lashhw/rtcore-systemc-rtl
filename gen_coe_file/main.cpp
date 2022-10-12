#include <fstream>
#include <iomanip>
#include "../third_party/happly.h"
#include "../custom_structs/bvh.hpp"

unsigned int float_as_unsigned_int(float x) {
    return *reinterpret_cast<unsigned int*>(&x);
}

int main() {
    happly::PLYData ply_data("../third_party/bun_zipper.ply");
    std::vector<std::array<double, 3>> v_pos = ply_data.getVertexPositions();
    std::vector<std::vector<size_t>> f_idx = ply_data.getFaceIndices<size_t>();

    std::vector<Triangle> triangles;
    for (int i = 0; i < f_idx.size(); i++) {
        const std::vector<size_t> &face = f_idx[i];
        triangles.emplace_back(Vec3(v_pos[face[0]][0], v_pos[face[0]][1], v_pos[face[0]][2]),
                               Vec3(v_pos[face[1]][0], v_pos[face[1]][1], v_pos[face[1]][2]),
                               Vec3(v_pos[face[2]][0], v_pos[face[2]][1], v_pos[face[2]][2]));
    }

    Bvh bvh(triangles);

    std::ofstream nodes_coe_file("nodes.coe");
    nodes_coe_file << "memory_initialization_radix = 16;\nmemory_initialization_vector =\n";
    for (int i = 0; i < bvh.num_nodes; i++) {
        const Bvh::Node &node = bvh.nodes[i];
        if (i != 0) nodes_coe_file << ",\n";
        for (int j = 0; j < 6; j++) {
            nodes_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(node.bbox.bounds[j]);
        }
        nodes_coe_file << std::setfill('0') << std::setw(8) << std::hex << node.num_trigs;
        nodes_coe_file << std::setfill('0') << std::setw(8) << std::hex << (node.is_leaf() ? node.first_trig_idx : node.left_node_idx);
    }
    nodes_coe_file << ';';

    std::ofstream triangles_coe_file("triangles.coe");
    triangles_coe_file << "memory_initialization_radix = 16;\nmemory_initialization_vector =\n";
    for (int i = 0; i < bvh.num_triangles; i++) {
        const Triangle &triangle = bvh.triangles[i];
        if (i != 0) triangles_coe_file << ",\n";
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.p0.x);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.p0.y);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.p0.z);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.e1.x);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.e1.y);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.e1.z);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.e2.x);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.e2.y);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.e2.z);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.n.x);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.n.y);
        triangles_coe_file << std::setfill('0') << std::setw(8) << std::hex << float_as_unsigned_int(triangle.n.z);
    }
    triangles_coe_file << ';';
}
