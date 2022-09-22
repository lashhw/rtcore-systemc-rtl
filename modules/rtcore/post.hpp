#ifndef RTCORE_SYSTEMC_POST_HPP
#define RTCORE_SYSTEMC_POST_HPP

template <int MAX_DEPTH>
SC_MODULE(POST) {
    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<int> s_ray_id;

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
    RD_POST_FIFO<MAX_DEPTH> post_fifo;

    // high-level objects
    RayState *ray_states;

    // internal signals
    sc_signal<bool> pf_s_valid;
    sc_signal<bool> pf_s_ready;
    sc_signal<int> pf_s_ray_id;
    sc_signal<bool> pf_m_valid;
    sc_signal<bool> pf_m_ready;
    sc_signal<int> pf_m_ray_id;

    sc_signal<bool> valid;

    SC_HAS_PROCESS(POST);
    POST(const sc_module_name &mn, RayState *ray_states)
        : sc_module(mn), post_fifo("post_fifo"), ray_states(ray_states) {
        post_fifo.s_valid(pf_s_valid);
        post_fifo.s_ready(pf_s_ready);
        post_fifo.s_ray_id(pf_s_ray_id);
        post_fifo.clk(clk);
        post_fifo.srstn(srstn);
        post_fifo.m_valid(pf_m_valid);
        post_fifo.m_ready(pf_m_ready);
        post_fifo.m_ray_id(pf_m_ray_id);

        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_ready)
        sensitive << pf_s_ready;

        SC_METHOD(update_m_valid)
        sensitive << m_ready << valid;

        SC_METHOD(update_pf_s_valid)
        sensitive << s_valid;

        SC_METHOD(update_pf_s_ray_id)
        sensitive << s_ray_id;

        SC_METHOD(update_pf_m_ready)
        sensitive << valid;
    }

    void main() {
        if (!srstn) {
            valid = false;
        } else {
            if (pf_m_valid && pf_m_ready) {
                valid = true;
                m_ray_id = pf_m_ray_id;
                m_hit = ray_states[pf_m_ray_id].hit;
                m_hit_trig_idx = ray_states[pf_m_ray_id].hit_trig_idx;
                m_t = ray_states[pf_m_ray_id].tmax;
                m_u = ray_states[pf_m_ray_id].u;
                m_v = ray_states[pf_m_ray_id].v;
            } else if (m_valid && m_ready) {
                valid = false;
            }
        }
    }

    void update_s_ready() {
        s_ready = pf_s_ready;
    }

    void update_m_valid() {
        m_valid = (m_ready && valid);
    }

    void update_pf_s_valid() {
        pf_s_valid = s_valid;
    }

    void update_pf_s_ray_id() {
        pf_s_ray_id = s_ray_id;
    }

    void update_pf_m_ready() {
        pf_m_ready = !valid;
    }
};

#endif //RTCORE_SYSTEMC_POST_HPP
