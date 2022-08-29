#ifndef DEFAULT_SYSTEMC_TRV_HPP
#define DEFAULT_SYSTEMC_TRV_HPP

#define IDLE 0
#define INIT 1
#define BBOX 2
#define LEAF_PREP 3
#define LEAF 4
#define SWITCH 5
#define STEP 6

// TODO: this TRV unit works only when the root node of BVH is not leaf
SC_MODULE(TRV) {
    // ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<bool> start;
    sc_in<float> origin_x;
    sc_in<float> origin_y;
    sc_in<float> origin_z;
    sc_in<float> dir_x;
    sc_in<float> dir_y;
    sc_in<float> dir_z;
    sc_in<float> tmax;
    sc_out<bool> done;  // wire
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
    sc_signal<int> stk_size;
    sc_signal<int> stk_data[Bvh::BVH_MAX_DEPTH-1];
    sc_signal<int> left_node_idx;
    sc_signal<int> right_node_idx;  // wire
    sc_signal<int> curr_node_idx;  // wire
    sc_signal<float> entry_left;
    sc_signal<float> entry_right;
    sc_signal<bool> curr_left;
    sc_signal<int> curr_trig_idx;
    sc_signal<int> last_trig_idx;
    sc_signal<bool> ist_isected;  // wire
    sc_signal<float> ist_t;  // wire
    sc_signal<float> ist_u;  // wire
    sc_signal<float> ist_v;  // wire
    sc_signal<bool> octant_x;
    sc_signal<bool> octant_y;
    sc_signal<bool> octant_z;
    sc_signal<float> inv_dir_x;
    sc_signal<float> inv_dir_y;
    sc_signal<float> inv_dir_z;
    sc_signal<float> scaled_origin_x;
    sc_signal<float> scaled_origin_y;
    sc_signal<float> scaled_origin_z;

    SC_HAS_PROCESS(TRV);
    TRV(sc_module_name mn, Bvh *bvh) : sc_module(mn), ist("ist", bvh), bvh(bvh) {
        ist.clk(clk);
        ist.reset(reset);
        ist.trig_idx(curr_trig_idx);
        ist.origin_x(origin_x);
        ist.origin_y(origin_y);
        ist.origin_z(origin_z);
        ist.dir_x(dir_x);
        ist.dir_y(dir_y);
        ist.dir_z(dir_z);
        ist.tmax(t);
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
        sensitive << curr_left << left_node_idx << right_node_idx;
    }

    void main() {
        if (state == INIT) {
            isected = false;
            t = tmax;
            stk_size = 0;
            left_node_idx = bvh->nodes[0].left_node_idx;
            entry_left = FLT_MAX;
            entry_right = FLT_MAX;
            curr_left = true;
            octant_x = dir_x < 0;
            octant_y = dir_y < 0;
            octant_z = dir_z < 0;
            float inv_dir_x_tmp = 1.f / ((fabsf(dir_x) < FLT_EPSILON) ? copysignf(FLT_EPSILON, dir_x) : dir_x);
            float inv_dir_y_tmp = 1.f / ((fabsf(dir_y) < FLT_EPSILON) ? copysignf(FLT_EPSILON, dir_y) : dir_y);
            float inv_dir_z_tmp = 1.f / ((fabsf(dir_z) < FLT_EPSILON) ? copysignf(FLT_EPSILON, dir_z) : dir_z);
            inv_dir_x = inv_dir_x_tmp;
            inv_dir_y = inv_dir_y_tmp;
            inv_dir_z = inv_dir_z_tmp;
            scaled_origin_x = -origin_x * inv_dir_x_tmp;
            scaled_origin_y = -origin_y * inv_dir_y_tmp;
            scaled_origin_z = -origin_z * inv_dir_z_tmp;
        } else if (state == BBOX) {
            float *bounds = bvh->nodes[curr_node_idx].bbox.bounds;
            float entry_x = inv_dir_x * bounds[0 + octant_x] + scaled_origin_x;
            float entry_y = inv_dir_y * bounds[2 + octant_y] + scaled_origin_y;
            float entry_z = inv_dir_z * bounds[4 + octant_z] + scaled_origin_z;
            float entry = fmaxf(entry_x, fmaxf(entry_y, entry_z));
            float exit_x = inv_dir_x * bounds[1 - octant_x] + scaled_origin_x;
            float exit_y = inv_dir_y * bounds[3 - octant_y] + scaled_origin_y;
            float exit_z = inv_dir_z * bounds[5 - octant_z] + scaled_origin_z;
            float exit = fminf(exit_x, fminf(exit_y, exit_z));
            if (entry <= exit) {
                if (curr_left) entry_left = entry;
                else entry_right = entry;
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
