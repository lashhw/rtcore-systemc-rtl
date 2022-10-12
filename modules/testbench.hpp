#ifndef RTCORE_SYSTEMC_TESTBENCH_HPP
#define RTCORE_SYSTEMC_TESTBENCH_HPP

#include <fstream>
#include <unordered_map>
#include "raygen.hpp"
#include "rtcore/rtcore.hpp"
#include "shader.hpp"

SC_MODULE(TESTBENCH) {
    // parameters
    static constexpr int width = 600;
    static constexpr int height = 600;

    // submodules
    RAYGEN<width, height> raygen;
    RTCORE<4> rtcore;
    SHADER<width, height> shader;

    // high-level objects
    std::unordered_map<int, int> ray_id_to_pixel_idx;

    // internal signals
    sc_clock clk;
    sc_signal<bool> srstn;

    // RAYGEN-RTCORE
    sc_signal<bool> raygen_rtcore_valid;
    sc_signal<bool> raygen_rtcore_ready;
    sc_signal<float> raygen_rtcore_origin_x;
    sc_signal<float> raygen_rtcore_origin_y;
    sc_signal<float> raygen_rtcore_origin_z;
    sc_signal<float> raygen_rtcore_dir_x;
    sc_signal<float> raygen_rtcore_dir_y;
    sc_signal<float> raygen_rtcore_dir_z;
    sc_signal<float> raygen_rtcore_tmax;
    sc_signal<int> raygen_rtcore_ray_id;

    // RTCORE-SHADER
    sc_signal<bool> rtcore_shader_valid;
    sc_signal<bool> rtcore_shader_ready;
    sc_signal<int> rtcore_shader_ray_id;
    sc_signal<bool> rtcore_shader_hit;
    sc_signal<int> rtcore_shader_hit_trig_idx;
    sc_signal<float> rtcore_shader_t;
    sc_signal<float> rtcore_shader_u;
    sc_signal<float> rtcore_shader_v;

    // vcd file
    sc_trace_file *tf;

    SC_HAS_PROCESS(TESTBENCH);
    TESTBENCH(const sc_module_name &mn, Bvh *bvh)
        : sc_module(mn), raygen("raygen", &ray_id_to_pixel_idx),
          rtcore("rtcore", bvh), shader("shader", bvh, &ray_id_to_pixel_idx),
          clk("clk", 2, SC_PS) {
        // link RAYGEN
        raygen.clk(clk);
        raygen.srstn(srstn);
        raygen.m_valid(raygen_rtcore_valid);
        raygen.m_ready(raygen_rtcore_ready);
        raygen.m_origin_x(raygen_rtcore_origin_x);
        raygen.m_origin_y(raygen_rtcore_origin_y);
        raygen.m_origin_z(raygen_rtcore_origin_z);
        raygen.m_dir_x(raygen_rtcore_dir_x);
        raygen.m_dir_y(raygen_rtcore_dir_y);
        raygen.m_dir_z(raygen_rtcore_dir_z);
        raygen.m_tmax(raygen_rtcore_tmax);
        raygen.m_ray_id(raygen_rtcore_ray_id);

        // link RTCORE
        rtcore.s_valid(raygen_rtcore_valid);
        rtcore.s_ready(raygen_rtcore_ready);
        rtcore.s_origin_x(raygen_rtcore_origin_x);
        rtcore.s_origin_y(raygen_rtcore_origin_y);
        rtcore.s_origin_z(raygen_rtcore_origin_z);
        rtcore.s_dir_x(raygen_rtcore_dir_x);
        rtcore.s_dir_y(raygen_rtcore_dir_y);
        rtcore.s_dir_z(raygen_rtcore_dir_z);
        rtcore.s_tmax(raygen_rtcore_tmax);
        rtcore.s_ray_id(raygen_rtcore_ray_id);
        rtcore.clk(clk);
        rtcore.srstn(srstn);
        rtcore.m_valid(rtcore_shader_valid);
        rtcore.m_ready(rtcore_shader_ready);
        rtcore.m_ray_id(rtcore_shader_ray_id);
        rtcore.m_hit(rtcore_shader_hit);
        rtcore.m_hit_trig_idx(rtcore_shader_hit_trig_idx);
        rtcore.m_t(rtcore_shader_t);
        rtcore.m_u(rtcore_shader_u);
        rtcore.m_v(rtcore_shader_v);

        // link SHADER
        shader.s_valid(rtcore_shader_valid);
        shader.s_ready(rtcore_shader_ready);
        shader.s_ray_id(rtcore_shader_ray_id);
        shader.s_hit(rtcore_shader_hit);
        shader.s_hit_trig_idx(rtcore_shader_hit_trig_idx);
        shader.s_t(rtcore_shader_t);
        shader.s_u(rtcore_shader_u);
        shader.s_v(rtcore_shader_v);
        shader.clk(clk);
        shader.srstn(srstn);

        SC_THREAD(main)

        tf = sc_create_vcd_trace_file("wave");
        /*
        sc_trace(tf, clk, "clk");
        sc_trace(tf, srstn, "srstn");
        sc_trace(tf, raygen.m_valid, "raygen.m_valid");
        sc_trace(tf, rtcore.rd.s_alloc_ready, "rtcore.rd.s_alloc_ready");
        sc_trace(tf, rtcore.rd.s_alloc_ray_id, "rtcore.rd.s_alloc_ray_id");
        sc_trace(tf, rtcore.rd.m_valid, "rtcore.rd.m_valid");
        sc_trace(tf, rtcore.rd.m_ray_id, "rtcore.rd.m_ray_id");
        sc_trace(tf, rtcore.trv.s_ready, "rtcore.trv.s_ready");
        sc_trace(tf, rtcore.trv.m_list_valid, "rtcore.trv.m_list_valid");
        sc_trace(tf, rtcore.trv.m_list_ray_id, "rtcore.trv.m_list_ray_id");
        sc_trace(tf, rtcore.trv.m_list_node_a_idx, "rtcore.trv.m_list_node_a_idx");
        sc_trace(tf, rtcore.trv.m_list_node_b_valid, "rtcore.trv.m_list_node_b_valid");
        sc_trace(tf, rtcore.trv.m_list_node_b_idx, "rtcore.trv.m_list_node_b_idx");
        sc_trace(tf, rtcore.trv.m_post_valid, "rtcore.trv.m_post_valid");
        sc_trace(tf, rtcore.trv.m_post_ray_id, "rtcore.trv.m_post_ray_id");
        sc_trace(tf, rtcore.list.s_ready, "rtcore.list.s_ready");
        sc_trace(tf, rtcore.list.m_valid, "rtcore.list.m_valid");
        sc_trace(tf, rtcore.list.m_ray_id, "rtcore.list.m_ray_id");
        sc_trace(tf, rtcore.list.m_trig_idx, "rtcore.list.m_trig_idx");
        sc_trace(tf, rtcore.list.m_is_last_trig, "rtcore.list.m_is_last_trig");
        sc_trace(tf, rtcore.post.s_ready, "rtcore.post.s_ready");
        sc_trace(tf, rtcore.post.m_valid, "rtcore.post.m_valid");
        sc_trace(tf, rtcore.post.m_ray_id, "rtcore.post.m_ray_id");
        sc_trace(tf, rtcore.ist.m_valid, "rtcore.ist.m_valid");
        sc_trace(tf, rtcore.ist.m_ray_id, "rtcore.ist.m_ray_id");
        sc_trace(tf, shader.s_ready, "shader.s_ready");
         */
    }

    void main() {
        srstn = false;
        wait(9, SC_PS);
        srstn = true;
    }

    ~TESTBENCH() {
        sc_close_vcd_trace_file(tf);
    }
};

#endif //RTCORE_SYSTEMC_TESTBENCH_HPP
