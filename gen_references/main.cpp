#include "../third_party/happly.h"
#include "bvh/triangle.hpp"
#include "bvh/sweep_sah_builder.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"

using Vector3  = bvh::Vector3<float>;
using Triangle = bvh::Triangle<float>;
using Ray = bvh::Ray<float>;
using Bvh = bvh::Bvh<float>;

constexpr int width = 600;
constexpr int height = 600;
constexpr float origin_x = 0.f;
constexpr float origin_y = 0.1f;
constexpr float origin_z = 1.f;
constexpr float horizontal = 0.2f;
constexpr float vertical = 0.2f;

int main() {
    happly::PLYData ply_data("../third_party/bun_zipper.ply");
    std::vector<std::array<double, 3>> v_pos = ply_data.getVertexPositions();
    std::vector<std::vector<size_t>> f_idx = ply_data.getFaceIndices<size_t>();

    std::vector<Triangle> triangles;
    for (int i = 0; i < f_idx.size(); i++) {
        const std::vector<size_t> &face = f_idx[i];
        triangles.emplace_back(Vector3(v_pos[face[0]][0], v_pos[face[0]][1], v_pos[face[0]][2]),
                               Vector3(v_pos[face[1]][0], v_pos[face[1]][1], v_pos[face[1]][2]),
                               Vector3(v_pos[face[2]][0], v_pos[face[2]][1], v_pos[face[2]][2]));
    }

    Bvh bvh;
    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), triangles.size());

    bvh::SweepSahBuilder<Bvh> builder(bvh);
    builder.build(global_bbox, bboxes.get(), centers.get(), triangles.size());

    bvh::ClosestPrimitiveIntersector<Bvh, Triangle> primitive_intersector(bvh, triangles.data());
    bvh::SingleRayTraverser<Bvh> traverser(bvh);

    std::ofstream image_file("image_reference.ppm");
    std::ofstream intersection_file("intersection_reference.txt");
    image_file << "P3\n" << width << ' ' << height << "\n255\n";

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float dir_x = (-0.1f + horizontal * j / width) - origin_x;
            float dir_y = (0.2f - vertical * i / height) - origin_y;
            float dir_z = -1.f;
            Ray ray(
                Vector3(origin_x, origin_y, origin_z),
                Vector3(dir_x, dir_y, dir_z),
                0.f
            );

            if (auto hit = traverser.traverse(ray, primitive_intersector)) {
                auto triangle_index = hit->primitive_index;
                auto intersection = hit->intersection;
                float r = triangles[triangle_index].n[0];
                float g = triangles[triangle_index].n[1];
                float b = triangles[triangle_index].n[2];
                float length = sqrtf(r * r + g * g + b * b);
                r = (r / length + 1.f) / 2.f;
                g = (g / length + 1.f) / 2.f;
                b = (b / length + 1.f) / 2.f;
                image_file << std::clamp(int(256.f * r), 0, 255) << ' '
                           << std::clamp(int(256.f * g), 0, 255) << ' '
                           << std::clamp(int(256.f * b), 0, 255) << '\n';
                intersection_file << intersection.t << ' '
                                  << intersection.u << ' '
                                  << intersection.v << '\n';
            } else {
                image_file << "0 0 0\n";
                intersection_file << "-1 -1 -1\n";
            }
        }
    }
}