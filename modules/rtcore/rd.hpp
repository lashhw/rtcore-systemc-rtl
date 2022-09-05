#ifndef RTCORE_SYSTEMC_RD_HPP
#define RTCORE_SYSTEMC_RD_HPP

#include "trv.hpp"

template<int MAX_WORKING_RAYS>
SC_MODULE(RD) {
    // ports
    sc_in<bool> s_alloc_valid;
    sc_out<bool> s_alloc_ready;
    sc_in<float> s_origin_x;
    sc_in<float> s_origin_y;
    sc_in<float> s_origin_z;
    sc_in<float> s_dir_x;
    sc_in<float> s_dir_y;
    sc_in<float> s_dir_z;
    sc_in<float> s_tmax;
    sc_out<int> s_alloc_ray_id;

    sc_in<bool> s_release_valid;
    sc_in<int> s_release_ray_id;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<int> m_ray_id;

    // high-level objects
    RayState *ray_states;

    // internal signals
    sc_signal<int> free_ray_id_data[MAX_WORKING_RAYS + 1];
    sc_signal<int> free_ray_id_front;
    sc_signal<int> free_ray_id_back;

    sc_signal<int> working_ray_id_data[MAX_WORKING_RAYS + 1];
    sc_signal<int> working_ray_id_front;
    sc_signal<int> working_ray_id_back;

    SC_HAS_PROCESS(RD);
    RD(const sc_module_name &mn, RayState *ray_states) : sc_module(mn), ray_states(ray_states) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_alloc_ready)
        sensitive << free_ray_id_front << free_ray_id_back;

        SC_METHOD(update_s_alloc_ray_id)
        for (int i = 0; i <= MAX_WORKING_RAYS; i++) sensitive << free_ray_id_data[i];
        sensitive << free_ray_id_front;

        SC_METHOD(update_m_valid)
        sensitive << working_ray_id_front << working_ray_id_back;

        SC_METHOD(update_m_ray_id)
        for (int i = 0; i <= MAX_WORKING_RAYS; i++) sensitive << working_ray_id_data[i];
        sensitive << working_ray_id_front;
    }

    void main() {
        if (!srstn) {
            for (int i = 0; i < MAX_WORKING_RAYS; i++) free_ray_id_data[i] = i;
            free_ray_id_front = 0;
            free_ray_id_back = MAX_WORKING_RAYS;
            working_ray_id_front = 0;
            working_ray_id_back = 0;
        } else {
            if (s_alloc_valid && s_alloc_ready) {
                ray_states[s_alloc_ray_id].origin_x = s_origin_x;
                ray_states[s_alloc_ray_id].origin_y = s_origin_y;
                ray_states[s_alloc_ray_id].origin_z = s_origin_z;
                ray_states[s_alloc_ray_id].dir_x = s_dir_x;
                ray_states[s_alloc_ray_id].dir_y = s_dir_y;
                ray_states[s_alloc_ray_id].dir_z = s_dir_z;
                ray_states[s_alloc_ray_id].tmax = s_tmax;

                ray_states[s_alloc_ray_id].left_node_idx = 1;
                ray_states[s_alloc_ray_id].finished = false;

                ray_states[s_alloc_ray_id].octant_x = s_dir_x < 0;
                ray_states[s_alloc_ray_id].octant_y = s_dir_y < 0;
                ray_states[s_alloc_ray_id].octant_z = s_dir_z < 0;
                float inv_dir_x_tmp = 1.f / ((fabsf(s_dir_x) < FLT_EPSILON) ? copysignf(FLT_EPSILON, s_dir_x) : s_dir_x);
                float inv_dir_y_tmp = 1.f / ((fabsf(s_dir_y) < FLT_EPSILON) ? copysignf(FLT_EPSILON, s_dir_y) : s_dir_y);
                float inv_dir_z_tmp = 1.f / ((fabsf(s_dir_z) < FLT_EPSILON) ? copysignf(FLT_EPSILON, s_dir_z) : s_dir_z);
                ray_states[s_alloc_ray_id].inv_dir_x = inv_dir_x_tmp;
                ray_states[s_alloc_ray_id].inv_dir_y = inv_dir_y_tmp;
                ray_states[s_alloc_ray_id].inv_dir_z = inv_dir_z_tmp;
                ray_states[s_alloc_ray_id].scaled_origin_x = -s_origin_x * inv_dir_x_tmp;
                ray_states[s_alloc_ray_id].scaled_origin_y = -s_origin_y * inv_dir_y_tmp;
                ray_states[s_alloc_ray_id].scaled_origin_z = -s_origin_z * inv_dir_z_tmp;

                ray_states[s_alloc_ray_id].hit = false;

                // pop from free_ray_id
                free_ray_id_front = (free_ray_id_front + 1) % (MAX_WORKING_RAYS + 1);

                // push to working_ray_id
                working_ray_id_data[working_ray_id_back] = s_alloc_ray_id;
                working_ray_id_back = (working_ray_id_back + 1) % (MAX_WORKING_RAYS + 1);
            }
            if (s_release_valid) {
                // push to free_ray_id
                free_ray_id_data[free_ray_id_back] = s_release_ray_id;
                free_ray_id_back = (free_ray_id_back + 1) % (MAX_WORKING_RAYS + 1);
            }
            if (m_valid && m_ready) {
                // pop from working_ray_id
                working_ray_id_front = (working_ray_id_front + 1) % (MAX_WORKING_RAYS + 1);
            }
        }
    }

    void update_s_alloc_ready() {
        s_alloc_ready = (free_ray_id_front != free_ray_id_back);
    }

    void update_s_alloc_ray_id() {
        s_alloc_ray_id = free_ray_id_data[free_ray_id_front];
    }

    void update_m_valid() {
        m_valid = (working_ray_id_front != working_ray_id_back);
    }

    void update_m_ray_id() {
        m_ray_id = working_ray_id_data[working_ray_id_front];
    }
};

#endif //RTCORE_SYSTEMC_RD_HPP
