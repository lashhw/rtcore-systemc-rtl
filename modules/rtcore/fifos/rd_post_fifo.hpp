#ifndef RTCORE_SYSTEMC_RD_POST_FIFO_HPP
#define RTCORE_SYSTEMC_RD_POST_FIFO_HPP

template<int MaxDepth, bool FillWhenReset = false>
SC_MODULE(RD_POST_FIFO) {
    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<int> s_ray_id;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<int> m_ray_id;

    // internal states
    sc_signal<int> ray_id[MaxDepth + 1];
    sc_signal<int> front;
    sc_signal<int> back;

    SC_CTOR(RD_POST_FIFO) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_ready)
        sensitive << front << back;

        SC_METHOD(update_m_valid)
        sensitive << front << back;

        SC_METHOD(update_m_ray_id)
        for (int i = 0; i <= MaxDepth; i++) sensitive << ray_id[i];
        sensitive << front;
    }

    void main() {
        if (!srstn) {
            if constexpr (FillWhenReset) {
                front = 0;
                back = MaxDepth;
                for (int i = 0; i < MaxDepth; i++) ray_id[i] = i;
            } else {
                front = 0;
                back = 0;
            }
        } else {
            if (s_valid && s_ready) {
                ray_id[back] = s_ray_id;
                back = (back + 1) % (MaxDepth + 1);
            }
            if (m_valid && m_ready) {
                front = (front + 1) % (MaxDepth + 1);
            }
        }
    }

    void update_s_ready() {
        s_ready = ((back + 1) % (MaxDepth + 1) != front);
    }

    void update_m_valid() {
        m_valid = (front != back);
    }

    void update_m_ray_id() {
        m_ray_id = ray_id[front];
    }
};

#endif //RTCORE_SYSTEMC_RD_POST_FIFO_HPP
