#ifndef RTCORE_SYSTEMC_LIST_HPP
#define RTCORE_SYSTEMC_LIST_HPP

#include "fifos/list_fifo.hpp"

template<int MAX_DEPTH>
SC_MODULE(LIST) {
    // send_state definitions
    static constexpr int IDLE = 0;
    static constexpr int LOAD = 1;
    static constexpr int SEND = 2;

    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<int> s_ray_id;
    sc_in<int> s_node_a_idx;
    sc_in<bool> s_node_b_valid;
    sc_in<int> s_node_b_idx;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_out<int> m_ray_id;
    sc_out<int> m_trig_idx;
    sc_out<bool> m_is_last_trig;

    // submodules
    LIST_FIFO<MAX_DEPTH> list_fifo;

    // high-level objects
    Bvh *bvh;

    // internal signals
    sc_signal<bool> lf_s_valid;
    sc_signal<bool> lf_s_ready;
    sc_signal<int> lf_s_ray_id;
    sc_signal<int> lf_s_node_idx;
    sc_signal<bool> lf_s_is_last_node;
    sc_signal<bool> lf_m_valid;
    sc_signal<bool> lf_m_ready;
    sc_signal<int> lf_m_ray_id;
    sc_signal<int> lf_m_node_idx;
    sc_signal<bool> lf_m_is_last_node;

    sc_signal<bool> recv_node_a;
    sc_signal<int> recv_ray_id;
    sc_signal<int> recv_node_b_idx;

    sc_signal<int> send_state;
    sc_signal<int> send_ray_id;
    sc_signal<int> send_node_idx;
    sc_signal<bool> send_is_last_node;
    sc_signal<int> send_last_trig_idx;

    SC_HAS_PROCESS(LIST);
    LIST(const sc_module_name &mn, Bvh *bvh)
        : sc_module(mn), list_fifo("list_fifo"), bvh(bvh) {
        // link LIST_FIFO
        list_fifo.s_valid(lf_s_valid);
        list_fifo.s_ready(lf_s_ready);
        list_fifo.s_ray_id(lf_s_ray_id);
        list_fifo.s_node_idx(lf_s_node_idx);
        list_fifo.s_is_last_node(lf_s_is_last_node);
        list_fifo.clk(clk);
        list_fifo.srstn(srstn);
        list_fifo.m_valid(lf_m_valid);
        list_fifo.m_ready(lf_m_ready);
        list_fifo.m_ray_id(lf_m_ray_id);
        list_fifo.m_node_idx(lf_m_node_idx);
        list_fifo.m_is_last_node(lf_m_is_last_node);

        SC_METHOD(recv)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(send)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_ready)
        sensitive << recv_node_a << lf_s_ready;

        SC_METHOD(update_m_valid)
        sensitive << send_state;

        SC_METHOD(update_m_is_last_trig)
        sensitive << send_is_last_node << m_trig_idx << send_last_trig_idx;

        SC_METHOD(update_lf_s_valid)
        sensitive << recv_node_a << s_valid;

        SC_METHOD(update_lf_s_ray_id)
        sensitive << recv_node_a << s_ray_id << recv_ray_id;

        SC_METHOD(update_lf_s_node_idx)
        sensitive << recv_node_a << s_node_a_idx << recv_node_b_idx;

        SC_METHOD(update_lf_s_is_last_node)
        sensitive << recv_node_a << s_node_b_valid;

        SC_METHOD(update_lf_m_ready)
        sensitive << send_state;
    }

    void recv() {
        if (!srstn) {
            recv_node_a = true;
        } else {
            if (lf_s_valid && lf_s_ready) {
                if (!recv_node_a) recv_node_a = true;
                else if (s_node_b_valid) {
                    recv_node_a = false;
                    recv_ray_id = s_ray_id;
                    recv_node_b_idx = s_node_b_idx;
                }
            }
        }
    }

    void send() {
        if (!srstn) {
            send_state = IDLE;
        } else {
            if (send_state == IDLE) {
                if (lf_m_valid && lf_m_ready) {
                    m_ray_id = lf_m_ray_id;
                    send_node_idx = lf_m_node_idx;
                    send_is_last_node = lf_m_is_last_node;

                    // update send_state
                    send_state = LOAD;
                }
            } else if (send_state == LOAD) {
                int first_trig_idx = bvh->nodes[send_node_idx].first_trig_idx;
                m_trig_idx = first_trig_idx;
                send_last_trig_idx = first_trig_idx + bvh->nodes[send_node_idx].num_trigs - 1;

                // update send_state
                send_state = SEND;
            } else if (send_state == SEND) {
                m_trig_idx = m_trig_idx + 1;

                // update send_state
                if (m_trig_idx == send_last_trig_idx) send_state = IDLE;
            }
        }
    }

    void update_s_ready() {
        s_ready = recv_node_a && lf_s_ready;
    }

    void update_m_valid() {
        m_valid = (send_state == SEND);
    }

    void update_m_is_last_trig() {
        m_is_last_trig = send_is_last_node && m_trig_idx == send_last_trig_idx;
    }

    void update_lf_s_valid() {
        lf_s_valid = !recv_node_a || s_valid;
    }

    void update_lf_s_ray_id() {
        lf_s_ray_id = recv_node_a ? s_ray_id : recv_ray_id;
    }

    void update_lf_s_node_idx() {
        lf_s_node_idx = recv_node_a ? s_node_a_idx : recv_node_b_idx;
    }

    void update_lf_s_is_last_node() {
        lf_s_is_last_node = !recv_node_a || !s_node_b_valid;
    }

    void update_lf_m_ready() {
        lf_m_ready = (send_state == IDLE);
    }
};

#endif //RTCORE_SYSTEMC_LIST_HPP
