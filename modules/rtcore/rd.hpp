#ifndef RTCORE_SYSTEMC_RD_HPP
#define RTCORE_SYSTEMC_RD_HPP

#include "fifos/rd_post_fifo.hpp"

template<int MaxWorkingRays>
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

    sc_in<bool> s_resume_valid;
    sc_in<int> s_resume_ray_id;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<int> m_ray_id;

    // submodules
    RD_POST_FIFO<MaxWorkingRays, true> free_fifo;
    RD_POST_FIFO<MaxWorkingRays, false> working_fifo;

    // high-level objects
    RayState *ray_states;

    // internal signals
    sc_signal<bool> ff_s_valid;
    sc_signal<bool> ff_s_ready;
    sc_signal<int> ff_s_ray_id;
    sc_signal<bool> ff_m_valid;
    sc_signal<bool> ff_m_ready;
    sc_signal<int> ff_m_ray_id;

    sc_signal<bool> wf_s_valid;
    sc_signal<bool> wf_s_ready;
    sc_signal<int> wf_s_ray_id;
    sc_signal<bool> wf_m_valid;
    sc_signal<bool> wf_m_ready;
    sc_signal<int> wf_m_ray_id;

    SC_HAS_PROCESS(RD);
    RD(const sc_module_name &mn, RayState *ray_states)
        : sc_module(mn), free_fifo("free_fifo"),
          working_fifo("working_fifo"), ray_states(ray_states) {
        free_fifo.s_valid(ff_s_valid);
        free_fifo.s_ready(ff_s_ready);
        free_fifo.s_ray_id(ff_s_ray_id);
        free_fifo.clk(clk);
        free_fifo.srstn(srstn);
        free_fifo.m_valid(ff_m_valid);
        free_fifo.m_ready(ff_m_ready);
        free_fifo.m_ray_id(ff_m_ray_id);

        working_fifo.s_valid(wf_s_valid);
        working_fifo.s_ready(wf_s_ready);
        working_fifo.s_ray_id(wf_s_ray_id);
        working_fifo.clk(clk);
        working_fifo.srstn(srstn);
        working_fifo.m_valid(wf_m_valid);
        working_fifo.m_ready(wf_m_ready);
        working_fifo.m_ray_id(wf_m_ray_id);

        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_alloc_ready)
        sensitive << ff_m_valid << s_resume_valid;

        SC_METHOD(update_s_alloc_ray_id)
        sensitive << ff_m_ray_id;

        SC_METHOD(update_m_valid)
        sensitive << wf_m_valid;

        SC_METHOD(update_m_ray_id)
        sensitive << wf_m_ray_id;

        SC_METHOD(update_ff_s_valid)
        sensitive << s_release_valid;

        SC_METHOD(update_ff_s_ray_id)
        sensitive << s_release_ray_id;

        SC_METHOD(update_ff_m_ready)
        sensitive << s_alloc_valid << s_alloc_ready;

        SC_METHOD(update_wf_s_valid)
        sensitive << s_resume_valid << s_alloc_valid << s_alloc_ready;

        SC_METHOD(update_wf_s_ray_id)
        sensitive << s_resume_valid << s_resume_ray_id << s_alloc_ray_id;

        SC_METHOD(update_wf_m_ready)
        sensitive << m_ready;
    }

    void main() {
        if (srstn)  {
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
            }
        }
    }

    void update_s_alloc_ready() {
        s_alloc_ready = (ff_m_valid && !s_resume_valid);
    }

    void update_s_alloc_ray_id() {
        s_alloc_ray_id = ff_m_ray_id;
    }

    void update_m_valid() {
        m_valid = wf_m_valid;
    }

    void update_m_ray_id() {
        m_ray_id = wf_m_ray_id;
    }

    void update_ff_s_valid() {
        ff_s_valid = s_release_valid;
    }

    void update_ff_s_ray_id() {
        ff_s_ray_id = s_release_ray_id;
    }

    void update_ff_m_ready() {
        ff_m_ready = (s_alloc_valid && s_alloc_ready);
    }

    void update_wf_s_valid() {
        wf_s_valid = (s_resume_valid || (s_alloc_valid && s_alloc_ready));
    }

    void update_wf_s_ray_id() {
        wf_s_ray_id = (s_resume_valid ? s_resume_ray_id : s_alloc_ray_id);
    }

    void update_wf_m_ready() {
        wf_m_ready = m_ready;
    }
};

#endif //RTCORE_SYSTEMC_RD_HPP
