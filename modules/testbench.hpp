#ifndef DEFAULT_SYSTEMC_TESTBENCH_HPP
#define DEFAULT_SYSTEMC_TESTBENCH_HPP

#include "ist.hpp"
#include "trv.hpp"
#include "../custom_structs/ray.hpp"
#include "../custom_structs/intersection.hpp"

SC_MODULE(Testbench) {
    // signals
    sc_clock clk;
    sc_signal<bool> reset;
    sc_signal<bool> start;
    sc_signal<Ray> ray;
    sc_signal<bool> done;
    sc_signal<bool> isected;
    sc_signal<float> t;
    sc_signal<float> u;
    sc_signal<float> v;
    sc_signal<int> trig_idx;

    sc_signal<int> test;

    // submodules
    TRV trv;

    // vcd file
    sc_trace_file* tf;

    SC_HAS_PROCESS(Testbench);
    Testbench(sc_module_name mn, Bvh *bvh)
        : sc_module(mn), clk("clk", 2, SC_PS), trv("trv", bvh) {
        trv.clk(clk);
        trv.reset(reset);
        trv.start(start);
        trv.ray(ray);
        trv.done(done);
        trv.isected(isected);
        trv.t(t);
        trv.u(u);
        trv.v(v);
        trv.trig_idx(trig_idx);

        SC_THREAD(main);

        tf = sc_create_vcd_trace_file("wave");
        sc_trace(tf, clk, "clk");
        sc_trace(tf, reset, "reset");
        sc_trace(tf, start, "start");
        sc_trace(tf, ray, "ray");
        sc_trace(tf, done, "done");
        sc_trace(tf, isected, "isected");
        sc_trace(tf, t, "t");
        sc_trace(tf, u, "u");
        sc_trace(tf, v, "v");
        sc_trace(tf, trig_idx, "trig_idx");

        sc_trace(tf, trv.state, "trv.state");
        sc_trace(tf, trv.entry_left, "trv.entry_left");
        sc_trace(tf, trv.entry_right, "trv.entry_right");
        sc_trace(tf, trv.curr_node_idx, "trv.curr_node_idx");

        sc_trace(tf, trv.ist.isected, "trv.ist.isected");
        sc_trace(tf, trv.ist.t, "trv.ist.t");
        sc_trace(tf, trv.ist.u, "trv.ist.u");
        sc_trace(tf, trv.ist.v, "trv.ist.v");
    }

    ~Testbench() {
        sc_close_vcd_trace_file(tf);
    }

    void main() {
        reset = false;
        start = false;
        ray = Ray(Vec3(0.f, 0.1f, 1.f), Vec3(0.f, 0.0f, -1.f));
        wait(5, SC_PS);
        reset = true;
        wait(2, SC_PS);
        start.write(true);
        wait(2, SC_PS);
        start = false;
    }
};


#endif //DEFAULT_SYSTEMC_TESTBENCH_HPP
