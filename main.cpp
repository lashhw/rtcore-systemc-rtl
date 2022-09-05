#include <systemc>
using namespace sc_core;
using namespace sc_dt;

#include "third_party/happly.h"
#include "custom_structs/bvh.hpp"
#include "modules/testbench.hpp"

Bvh get_bvh() {
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

    return Bvh(triangles);
}

int sc_main(int, char*[]) {
    Bvh bvh = get_bvh();
    TESTBENCH tb("tb", &bvh);
    sc_start(100000000, SC_PS);
    return 0;
}