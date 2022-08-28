#ifndef DEFAULT_SYSTEMC_TRV_HPP
#define DEFAULT_SYSTEMC_TRV_HPP

#include "../custom_structs/aabb_intersector.hpp"

#define IDLE 0
#define INIT 1
#define BBOX 2
#define LEAF_PREP 3
#define LEAF 4
#define SWITCH 5
#define STEP 6

SC_MODULE(TRV) {
    // ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<bool> start;
    sc_in<Ray> ray;
    sc_out<bool> done;
    sc_out<bool> isected;
    sc_out<float> t;
    sc_out<float> u;
    sc_out<float> v;
    sc_out<int> trig_idx;

    // submodules
    IST ist;

    // internal states
    Bvh *bvh;
    sc_signal<int> state;
    AABBIntersector aabb_intersector;
    sc_signal<int> stk_size;
    sc_signal<int> stk_data[Bvh::BVH_MAX_DEPTH];
    sc_signal<int> left_node_idx;
    sc_signal<int> right_node_idx;  // wire
    sc_signal<int> curr_node_idx;  // wire
    sc_signal<Ray> curr_ray;
    sc_signal<float> entry_left;
    sc_signal<float> entry_right;
    sc_signal<bool> curr_left;
    sc_signal<int> curr_trig_idx;
    sc_signal<int> last_trig_idx;
    sc_signal<bool> ist_isected;
    sc_signal<float> ist_t;
    sc_signal<float> ist_u;
    sc_signal<float> ist_v;

    SC_HAS_PROCESS(TRV);
    TRV(sc_module_name mn, Bvh *bvh) : sc_module(mn), ist("ist", bvh), bvh(bvh) {
        ist.clk(clk);
        ist.reset(reset);
        ist.ray(curr_ray);
        ist.trig_idx(curr_trig_idx);
        ist.isected(ist_isected);
        ist.t(ist_t);
        ist.u(ist_u);
        ist.v(ist_v);

        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_state)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_done)
        sensitive << state;

        SC_METHOD(update_right_node_idx)
        sensitive << left_node_idx;

        SC_METHOD(update_curr_node_idx)
        sensitive << curr_left;
    }

    void main() {
        if (state == INIT) {
            aabb_intersector = AABBIntersector(ray);
            stk_size = 0;
            left_node_idx = bvh->nodes[0].left_node_idx;
            curr_ray = ray;
            entry_left = FLT_MAX;
            entry_right = FLT_MAX;
            curr_left = true;
        } else if (state == BBOX) {
            float tmp_entry;
            if (aabb_intersector.intersect(bvh->nodes[curr_node_idx].bbox, tmp_entry)) {
                if (curr_left) entry_left = tmp_entry;
                else entry_right = tmp_entry;
            }
        } else if (state == LEAF_PREP) {
            int first_trig_idx = bvh->nodes[curr_node_idx].first_trig_idx;
            if (curr_left) entry_left = FLT_MAX;
            else entry_right = FLT_MAX;
            curr_trig_idx = first_trig_idx;
            last_trig_idx = first_trig_idx + bvh->nodes[curr_node_idx].num_trigs - 1;
        } else if (state == LEAF) {
            if (ist_isected) {
                isected = true;
                t = ist_t;
                u = ist_u;
                v = ist_v;
                trig_idx = curr_trig_idx;
                Ray tmp_ray = curr_ray;
                tmp_ray.tmax = ist_t;
                curr_ray = tmp_ray;
            }
            curr_trig_idx = curr_trig_idx + 1;
        } else if (state == SWITCH) {
            curr_left = false;
        } else if (state == STEP) {
            if (entry_left != FLT_MAX) {
                if (entry_right != FLT_MAX) {
                    if (entry_left > entry_right) {
                        stk_data[stk_size] = bvh->nodes[left_node_idx].left_node_idx;
                        stk_size = stk_size + 1;
                        left_node_idx = bvh->nodes[right_node_idx].left_node_idx;
                    } else {
                        stk_data[stk_size] = bvh->nodes[right_node_idx].left_node_idx;
                        stk_size = stk_size + 1;
                        left_node_idx = bvh->nodes[left_node_idx].left_node_idx;
                    }
                } else {
                    left_node_idx = bvh->nodes[left_node_idx].left_node_idx;
                }
            } else if (entry_right != FLT_MAX) {
                left_node_idx = bvh->nodes[right_node_idx].left_node_idx;
            } else {
                if (stk_size != 0) {
                    left_node_idx = stk_data[stk_size - 1];
                    stk_size = stk_size - 1;
                }
            }
            curr_left = true;
            entry_left = FLT_MAX;
            entry_right = FLT_MAX;
        }
    }

    void update_state() {
        if (!reset) {
            state = IDLE;
        } else if (state == IDLE) {
            if (start) state = INIT;
            else state = IDLE;
        } else if (state == INIT) {
            state = BBOX;
        } else if (state == BBOX) {
            if (bvh->nodes[curr_node_idx].is_leaf()) state = LEAF_PREP;
            else state = SWITCH;
        } else if (state == LEAF_PREP) {
            state = LEAF;
        } else if (state == LEAF) {
            if (curr_trig_idx == last_trig_idx) state = SWITCH;
            else state = LEAF;
        } else if (state == SWITCH) {
            if (curr_left) state = BBOX;
            else state = STEP;
        } else if (state == STEP) {
            if (entry_left == FLT_MAX && entry_right == FLT_MAX && stk_size == 0) state = IDLE;
            else state = BBOX;
        }
    }

    void update_done() {
        done = (state == IDLE);
    }

    void update_right_node_idx() {
        right_node_idx = left_node_idx + 1;
    }

    void update_curr_node_idx() {
        if (curr_left) curr_node_idx = left_node_idx;
        else curr_node_idx = right_node_idx;
    }
};

#endif //DEFAULT_SYSTEMC_TRV_HPP
