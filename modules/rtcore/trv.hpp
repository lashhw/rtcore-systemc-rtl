#ifndef RTCORE_SYSTEMC_TRV_HPP
#define RTCORE_SYSTEMC_TRV_HPP

// TODO: this TRV unit works only when the root node of the BVH is not leaf
template<int MAX_WORKING_RAYS>
SC_MODULE(TRV) {
    // state definitions
    static constexpr int IDLE = 0;
    static constexpr int LOAD = 1;
    static constexpr int BBOX_LOAD = 2;
    static constexpr int BBOX = 3;
    static constexpr int NODE_LOAD = 4;
    static constexpr int STEP = 5;
    static constexpr int STORE = 6;
    static constexpr int LIST_LOAD = 7;
    static constexpr int LIST_A = 8;
    static constexpr int LIST_B = 9;
    static constexpr int POST = 10;

    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<int> s_ray_id;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<int> m_ray_id;

    sc_out<bool> m_ist_valid;
    sc_out<int> m_ist_trig_idx;
    sc_out<bool> m_is_last_trig;

    sc_out<bool> m_pf_valid;
    sc_in<bool> m_pf_ready;

    // high-level objects
    Bvh *bvh;
    RayState *ray_states;

    // internal signals
    sc_signal<int> state;

    sc_signal<int> left_node_idx;
    sc_signal<int> right_node_idx;

    sc_signal<bool> octant_x;
    sc_signal<bool> octant_y;
    sc_signal<bool> octant_z;
    sc_signal<float> inv_dir_x;
    sc_signal<float> inv_dir_y;
    sc_signal<float> inv_dir_z;
    sc_signal<float> scaled_origin_x;
    sc_signal<float> scaled_origin_y;
    sc_signal<float> scaled_origin_z;

    sc_signal<float> left_bound_x_min;
    sc_signal<float> left_bound_x_max;
    sc_signal<float> left_bound_y_min;
    sc_signal<float> left_bound_y_max;
    sc_signal<float> left_bound_z_min;
    sc_signal<float> left_bound_z_max;
    sc_signal<bool> left_is_leaf;

    sc_signal<float> right_bound_x_min;
    sc_signal<float> right_bound_x_max;
    sc_signal<float> right_bound_y_min;
    sc_signal<float> right_bound_y_max;
    sc_signal<float> right_bound_z_min;
    sc_signal<float> right_bound_z_max;
    sc_signal<bool> right_is_leaf;

    sc_signal<bool> left_hit;
    sc_signal<bool> right_hit;
    sc_signal<float> left_entry;
    sc_signal<float> right_entry;

    sc_signal<int> left_node_left_node_idx;
    sc_signal<int> right_node_left_node_idx;

    sc_signal<int> old_left_node_idx;
    sc_signal<int> old_right_node_idx;
    sc_signal<int> stk_size[MAX_WORKING_RAYS];
    sc_signal<int> stk_data[MAX_WORKING_RAYS][Bvh::BVH_MAX_DEPTH - 1];
    sc_signal<int> finished;

    sc_signal<int> a_last_trig_idx;
    sc_signal<int> b_first_trig_idx;
    sc_signal<int> b_last_trig_idx;

    SC_HAS_PROCESS(TRV);
    TRV(sc_module_name mn, Bvh *bvh, RayState *ray_states)
        : sc_module(mn), bvh(bvh), ray_states(ray_states) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_ready)
        sensitive << state;

        SC_METHOD(update_m_ist_valid)
        sensitive << state;

        SC_METHOD(update_m_is_last_trig)
        sensitive << state << m_ist_trig_idx << b_last_trig_idx;

        SC_METHOD(update_m_post_valid)
        sensitive << state;

        SC_METHOD(update_right_node_idx)
        sensitive << left_node_idx;

        SC_METHOD(update_old_right_node_idx)
        sensitive << old_left_node_idx;
    }

    void main() {
        if (!srstn) {
            state = IDLE;
        } if (state == IDLE) {
            m_ray_id = s_ray_id;

            // update state
            if (s_valid) state = LOAD;
        } else if (state == LOAD) {
            int left_node_idx_tmp = ray_states[m_ray_id].left_node_idx;
            left_node_idx = left_node_idx_tmp;

            octant_x = ray_states[m_ray_id].octant_x;
            octant_y = ray_states[m_ray_id].octant_y;
            octant_z = ray_states[m_ray_id].octant_z;
            inv_dir_x = ray_states[m_ray_id].inv_dir_x;
            inv_dir_y = ray_states[m_ray_id].inv_dir_y;
            inv_dir_z = ray_states[m_ray_id].inv_dir_z;
            scaled_origin_x = ray_states[m_ray_id].scaled_origin_x;
            scaled_origin_y = ray_states[m_ray_id].scaled_origin_y;
            scaled_origin_z = ray_states[m_ray_id].scaled_origin_z;

            // update state
            if (ray_states[m_ray_id].finished) state = POST;
            else state = BBOX_LOAD;
        } else if (state == BBOX_LOAD) {
            float *left_bounds = bvh->nodes[left_node_idx].bbox.bounds;
            left_bound_x_min = left_bounds[0];
            left_bound_x_max = left_bounds[1];
            left_bound_y_min = left_bounds[2];
            left_bound_y_max = left_bounds[3];
            left_bound_z_min = left_bounds[4];
            left_bound_z_max = left_bounds[5];
            left_is_leaf = bvh->nodes[left_node_idx].is_leaf();

            float *right_bounds = bvh->nodes[right_node_idx].bbox.bounds;
            right_bound_x_min = right_bounds[0];
            right_bound_x_max = right_bounds[1];
            right_bound_y_min = right_bounds[2];
            right_bound_y_max = right_bounds[3];
            right_bound_z_min = right_bounds[4];
            right_bound_z_max = right_bounds[5];
            right_is_leaf = bvh->nodes[right_node_idx].is_leaf();

            // update state
            state = BBOX;
        } else if (state == BBOX) {
            float left_entry_x = inv_dir_x * (octant_x ? left_bound_x_max : left_bound_x_min) + scaled_origin_x;
            float left_entry_y = inv_dir_y * (octant_y ? left_bound_y_max : left_bound_y_min) + scaled_origin_y;
            float left_entry_z = inv_dir_z * (octant_z ? left_bound_z_max : left_bound_z_min) + scaled_origin_z;
            float left_entry_tmp = fmaxf(left_entry_x, fmaxf(left_entry_y, left_entry_z));
            float left_exit_x = inv_dir_x * (octant_x ? left_bound_x_min : left_bound_x_max) + scaled_origin_x;
            float left_exit_y = inv_dir_y * (octant_y ? left_bound_y_min : left_bound_y_max) + scaled_origin_y;
            float left_exit_z = inv_dir_z * (octant_z ? left_bound_z_min : left_bound_z_max) + scaled_origin_z;
            float left_exit = fminf(left_exit_x, fminf(left_exit_y, left_exit_z));

            left_hit = left_entry_tmp <= left_exit;
            left_entry = left_entry_tmp;

            float right_entry_x = inv_dir_x * (octant_x ? right_bound_x_max : right_bound_x_min) + scaled_origin_x;
            float right_entry_y = inv_dir_y * (octant_y ? right_bound_y_max : right_bound_y_min) + scaled_origin_y;
            float right_entry_z = inv_dir_z * (octant_z ? right_bound_z_max : right_bound_z_min) + scaled_origin_z;
            float right_entry_tmp = fmaxf(right_entry_x, fmaxf(right_entry_y, right_entry_z));
            float right_exit_x = inv_dir_x * (octant_x ? right_bound_x_min : right_bound_x_max) + scaled_origin_x;
            float right_exit_y = inv_dir_y * (octant_y ? right_bound_y_min : right_bound_y_max) + scaled_origin_y;
            float right_exit_z = inv_dir_z * (octant_z ? right_bound_z_min : right_bound_z_max) + scaled_origin_z;
            float right_exit = fminf(right_exit_x, fminf(right_exit_y, right_exit_z));

            right_hit = right_entry_tmp <= right_exit;
            right_entry = right_entry_tmp;

            // update state
            state = NODE_LOAD;
        } else if (state == NODE_LOAD) {
            left_node_left_node_idx = bvh->nodes[left_node_idx].left_node_idx;
            right_node_left_node_idx = bvh->nodes[right_node_idx].left_node_idx;

            // update state
            state = STEP;
        } else if (state == STEP) {
            old_left_node_idx = left_node_idx;

            bool left_valid = left_hit && !left_is_leaf;
            bool right_valid = right_hit && !right_is_leaf;
            if (left_valid) {
                if (right_valid) {
                    if (left_entry > right_entry) {
                        stk_data[m_ray_id][stk_size[m_ray_id]] = left_node_left_node_idx;
                        stk_size[m_ray_id] = stk_size[m_ray_id] + 1;
                        left_node_idx = right_node_left_node_idx;
                        finished = false;
                    } else {
                        stk_data[m_ray_id][stk_size[m_ray_id]] = right_node_left_node_idx;
                        stk_size[m_ray_id] = stk_size[m_ray_id] + 1;
                        left_node_idx = left_node_left_node_idx;
                        finished = false;
                    }
                } else {
                    left_node_idx = left_node_left_node_idx;
                    finished = false;
                }
            } else if (right_valid) {
                left_node_idx = right_node_left_node_idx;
                finished = false;
            } else {
                if (stk_size[m_ray_id] != 0) {
                    left_node_idx = stk_data[m_ray_id][stk_size[m_ray_id] - 1];
                    stk_size[m_ray_id] = stk_size[m_ray_id] - 1;
                    finished = false;
                } else {
                    finished = true;
                }
            }

            // update state
            if (!left_hit && !right_hit && stk_size[m_ray_id] == 0) state = POST;
            else if ((left_hit && left_is_leaf) || (right_hit && right_is_leaf)) state = STORE;
            else state = BBOX_LOAD;
        } else if (state == STORE) {
            ray_states[m_ray_id].left_node_idx = left_node_idx;
            ray_states[m_ray_id].finished = finished;

            // update state
            state = LIST_LOAD;
        } else if (state == LIST_LOAD) {
            if (left_hit && left_is_leaf) {
                if (right_hit && right_is_leaf) {
                    int a_first_trig_idx = bvh->nodes[old_left_node_idx].first_trig_idx;
                    m_ist_trig_idx = a_first_trig_idx;
                    a_last_trig_idx = a_first_trig_idx + bvh->nodes[old_left_node_idx].num_trigs - 1;
                    int b_first_trig_idx_tmp = bvh->nodes[old_right_node_idx].first_trig_idx;
                    b_first_trig_idx = b_first_trig_idx_tmp;
                    b_last_trig_idx = b_first_trig_idx_tmp + bvh->nodes[old_right_node_idx].num_trigs - 1;

                    // update state
                    state = LIST_A;
                } else {
                    int b_first_trig_idx_tmp = bvh->nodes[old_left_node_idx].first_trig_idx;
                    m_ist_trig_idx = b_first_trig_idx_tmp;
                    b_last_trig_idx = b_first_trig_idx_tmp + bvh->nodes[old_left_node_idx].num_trigs - 1;

                    // update state
                    state = LIST_B;
                }
            } else {
                int b_first_trig_idx_tmp = bvh->nodes[old_right_node_idx].first_trig_idx;
                m_ist_trig_idx = b_first_trig_idx_tmp;
                b_last_trig_idx = b_first_trig_idx_tmp + bvh->nodes[old_right_node_idx].num_trigs - 1;

                // update state
                state = LIST_B;
            }
        } else if (state == LIST_A) {
            if (m_ist_trig_idx == a_last_trig_idx) {
                m_ist_trig_idx = b_first_trig_idx;

                // update state
                state = LIST_B;
            } else {
                m_ist_trig_idx = m_ist_trig_idx + 1;
            }
        } else if (state == LIST_B) {
            m_ist_trig_idx = m_ist_trig_idx + 1;

            // update state
            if (m_ist_trig_idx == b_last_trig_idx) state = IDLE;
        } else if (state == POST) {
            // update state
            if (m_pf_ready) state = IDLE;
        }
    }

    void update_s_ready() {
        s_ready = (state == IDLE);
    }

    void update_m_ist_valid() {
        m_ist_valid = (state == LIST_A || state == LIST_B);
    }

    void update_m_is_last_trig() {
        m_is_last_trig = (state == LIST_B && m_ist_trig_idx == b_last_trig_idx);
    }

    void update_m_post_valid() {
        m_pf_valid = (state == POST);
    }

    void update_right_node_idx() {
        right_node_idx = left_node_idx + 1;
    }

    void update_old_right_node_idx() {
        old_right_node_idx = old_left_node_idx + 1;
    }
};

#endif //RTCORE_SYSTEMC_TRV_HPP
