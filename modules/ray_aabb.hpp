#ifndef RTCORE_SYSTEMC_RAY_AABB_HPP
#define RTCORE_SYSTEMC_RAY_AABB_HPP

SC_MODULE(RAY_AABB) {
    // ports
    sc_in<bool> clk;
    sc_in<int> curr_node_idx;
    sc_in<bool> octant_x;
    sc_in<bool> octant_y;
    sc_in<bool> octant_z;
    sc_in<float> inv_dir_x;
    sc_in<float> inv_dir_y;
    sc_in<float> inv_dir_z;
    sc_in<float> scaled_origin_x;
    sc_in<float> scaled_origin_y;
    sc_in<float> scaled_origin_z;
    sc_out<bool> isected;
    sc_out<float> entry;

    // internal states
    Bvh *bvh;

    SC_HAS_PROCESS(RAY_AABB);

    RAY_AABB(sc_module_name mn, Bvh *bvh) : sc_module(mn), bvh(bvh) {
        SC_METHOD(main);
        sensitive << curr_node_idx << octant_x << octant_y << octant_z << inv_dir_x << inv_dir_y << inv_dir_z
                  << scaled_origin_x << scaled_origin_y << scaled_origin_z;
    }

    void main() {
        float *bounds = bvh->nodes[curr_node_idx].bbox.bounds;
        float entry_x = inv_dir_x * bounds[0 + octant_x] + scaled_origin_x;
        float entry_y = inv_dir_y * bounds[2 + octant_y] + scaled_origin_y;
        float entry_z = inv_dir_z * bounds[4 + octant_z] + scaled_origin_z;
        float entry_tmp = fmaxf(entry_x, fmaxf(entry_y, entry_z));
        float exit_x = inv_dir_x * bounds[1 - octant_x] + scaled_origin_x;
        float exit_y = inv_dir_y * bounds[3 - octant_y] + scaled_origin_y;
        float exit_z = inv_dir_z * bounds[5 - octant_z] + scaled_origin_z;
        float exit = fminf(exit_x, fminf(exit_y, exit_z));

        isected = entry_tmp <= exit;
        entry = entry_tmp;
    }
};

#endif //RTCORE_SYSTEMC_RAY_AABB_HPP
