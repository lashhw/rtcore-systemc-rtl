#ifndef RTCORE_SYSTEMC_RAY_STATE_HPP
#define RTCORE_SYSTEMC_RAY_STATE_HPP

struct RayState {
    // ray data
    float origin_x;
    float origin_y;
    float origin_z;
    float dir_x;
    float dir_y;
    float dir_z;
    float tmax;

    // for TRV
    int left_node_idx;
    bool finished;

    // for ray-AABB intersection
    float octant_x;
    float octant_y;
    float octant_z;
    float inv_dir_x;
    float inv_dir_y;
    float inv_dir_z;
    float scaled_origin_x;
    float scaled_origin_y;
    float scaled_origin_z;

    // for IST
    bool hit;
    int hit_trig_idx;
    float u;
    float v;
};


#endif //RTCORE_SYSTEMC_RAY_STATE_HPP
