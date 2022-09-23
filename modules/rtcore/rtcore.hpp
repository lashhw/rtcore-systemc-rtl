#ifndef RTCORE_SYSTEMC_RTCORE_HPP
#define RTCORE_SYSTEMC_RTCORE_HPP

#include "../../custom_structs/ray_state.hpp"
#include "rd.hpp"
#include "trv.hpp"
#include "list.hpp"
#include "post.hpp"
#include "ist.hpp"

template<int MaxWorkingRays>
SC_MODULE(RTCORE) {
    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<float> s_origin_x;
    sc_in<float> s_origin_y;
    sc_in<float> s_origin_z;
    sc_in<float> s_dir_x;
    sc_in<float> s_dir_y;
    sc_in<float> s_dir_z;
    sc_in<float> s_tmax;
    sc_out<int> s_ray_id;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<int> m_ray_id;
    sc_out<bool> m_hit;
    sc_out<int> m_hit_trig_idx;
    sc_out<float> m_t;
    sc_out<float> m_u;
    sc_out<float> m_v;

    // submodules
    RD<MaxWorkingRays> rd;
    TRV<MaxWorkingRays> trv;
    LIST<MaxWorkingRays> list;
    POST<MaxWorkingRays> post;
    IST ist;

    // high-level objects
    Bvh *bvh;
    RayState ray_states[MaxWorkingRays];

    // internal signals
    // RD-IST
    sc_signal<bool> rd_ist_valid;
    sc_signal<int> rd_ist_ray_id;

    // RD-TRV
    sc_signal<bool> rd_trv_valid;
    sc_signal<bool> rd_trv_ready;
    sc_signal<int> rd_trv_ray_id;

    // TRV-LIST
    sc_signal<bool> trv_list_valid;
    sc_signal<bool> trv_list_ready;
    sc_signal<int> trv_list_ray_id;
    sc_signal<int> trv_list_node_a_idx;
    sc_signal<bool> trv_list_node_b_valid;
    sc_signal<int> trv_list_node_b_idx;

    // TRV-POST
    sc_signal<bool> trv_post_valid;
    sc_signal<bool> trv_post_ready;
    sc_signal<int> trv_post_ray_id;

    // LIST-IST
    sc_signal<bool> list_ist_valid;
    sc_signal<int> list_ist_ray_id;
    sc_signal<int> list_ist_trig_idx;
    sc_signal<bool> list_ist_is_last_trig;

    // vcd file
    sc_trace_file* tf;

    SC_HAS_PROCESS(RTCORE);
    RTCORE(const sc_module_name &mn, Bvh *bvh)
        : sc_module(mn), rd("rd", ray_states),
          trv("trv", bvh, ray_states), list("list", bvh),
          post("post", ray_states), ist("ist", bvh, ray_states),
          bvh(bvh) {
        // link RD
        rd.s_alloc_valid(s_valid);
        rd.s_alloc_ready(s_ready);
        rd.s_origin_x(s_origin_x);
        rd.s_origin_y(s_origin_y);
        rd.s_origin_z(s_origin_z);
        rd.s_dir_x(s_dir_x);
        rd.s_dir_y(s_dir_y);
        rd.s_dir_z(s_dir_z);
        rd.s_tmax(s_tmax);
        rd.s_alloc_ray_id(s_ray_id);
        rd.s_release_valid(m_valid);
        rd.s_release_ray_id(m_ray_id);
        rd.s_resume_valid(rd_ist_valid);
        rd.s_resume_ray_id(rd_ist_ray_id);
        rd.clk(clk);
        rd.srstn(srstn);
        rd.m_valid(rd_trv_valid);
        rd.m_ready(rd_trv_ready);
        rd.m_ray_id(rd_trv_ray_id);

        // link TRV
        trv.s_valid(rd_trv_valid);
        trv.s_ready(rd_trv_ready);
        trv.s_ray_id(rd_trv_ray_id);
        trv.clk(clk);
        trv.srstn(srstn);
        trv.m_list_valid(trv_list_valid);
        trv.m_list_ready(trv_list_ready);
        trv.m_list_ray_id(trv_list_ray_id);
        trv.m_list_node_a_idx(trv_list_node_a_idx);
        trv.m_list_node_b_valid(trv_list_node_b_valid);
        trv.m_list_node_b_idx(trv_list_node_b_idx);
        trv.m_post_valid(trv_post_valid);
        trv.m_post_ready(trv_post_ready);
        trv.m_post_ray_id(trv_post_ray_id);

        // link LIST
        list.s_valid(trv_list_valid);
        list.s_ready(trv_list_ready);
        list.s_ray_id(trv_list_ray_id);
        list.s_node_a_idx(trv_list_node_a_idx);
        list.s_node_b_valid(trv_list_node_b_valid);
        list.s_node_b_idx(trv_list_node_b_idx);
        list.clk(clk);
        list.srstn(srstn);
        list.m_valid(list_ist_valid);
        list.m_ray_id(list_ist_ray_id);
        list.m_trig_idx(list_ist_trig_idx);
        list.m_is_last_trig(list_ist_is_last_trig);

        // link POST
        post.s_valid(trv_post_valid);
        post.s_ready(trv_post_ready);
        post.s_ray_id(trv_post_ray_id);
        post.clk(clk);
        post.srstn(srstn);
        post.m_valid(m_valid);
        post.m_ready(m_ready);
        post.m_ray_id(m_ray_id);
        post.m_hit(m_hit);
        post.m_hit_trig_idx(m_hit_trig_idx);
        post.m_t(m_t);
        post.m_u(m_u);
        post.m_v(m_v);

        // link IST
        ist.s_valid(list_ist_valid);
        ist.s_ray_id(list_ist_ray_id);
        ist.s_trig_idx(list_ist_trig_idx);
        ist.s_is_last_trig(list_ist_is_last_trig);
        ist.clk(clk);
        ist.m_valid(rd_ist_valid);
        ist.m_ray_id(rd_ist_ray_id);
    }
};


#endif //RTCORE_SYSTEMC_RTCORE_HPP
