#include <random>
#include <algorithm>

#include <frei0r.h>

struct vhs
{
    unsigned int m_width;
    unsigned int m_height;

    unsigned int m_roll_position = 0;
    unsigned int m_roll_size;
    unsigned int m_roll_step;

    unsigned int m_chroma_shift;
    unsigned int m_scratch_count;

    std::mt19937 m_prng;
    
    std::uniform_int_distribution<unsigned int> m_right_shift;
    std::uniform_int_distribution<size_t>       m_scratch_position;
    std::uniform_int_distribution<unsigned int> m_scratch_length;

    vhs(unsigned int width, unsigned int height)
        : m_width(width), m_height(height),
          
          m_roll_size(height * 0.08),
          m_roll_step(height * 0.01),

          m_chroma_shift(width * 0.01),
          m_scratch_count(8),
          
          m_right_shift(width * 0.008, width * 0.01),
          m_scratch_position(0, size_t(width) * height),
          
          m_scratch_length(width * 0.01, width * 0.1)
        {}

    void process(const uint32_t *inframe, uint32_t *outframe)
    {
        unsigned int roll_end = std::min(m_roll_position + m_roll_size, m_height);

        for (unsigned int j = 0; j < m_height; ++j)
        {
            unsigned int shift = m_right_shift(m_prng);
            const size_t row = size_t(j) * m_width;

            size_t in_row_end = row + m_width - shift;
            for (size_t i = row; i < in_row_end; ++i)
            {
                const uint8_t *PIn_0 = reinterpret_cast<const uint8_t*>(inframe + i); // Y
                const uint8_t *PIn_1 = reinterpret_cast<const uint8_t*>(inframe + std::max<size_t>(i, m_chroma_shift) - m_chroma_shift); // U
                const uint8_t *PIn_2 = reinterpret_cast<const uint8_t*>(inframe + std::min(i + m_chroma_shift, in_row_end)); // V
                uint8_t *POut = reinterpret_cast<uint8_t*>(outframe + i + shift);

                // RGB to YUV, with half contrast reduction
                float Y =  .1495   * PIn_0[0] + .2935    * PIn_0[1] + .057     * PIn_0[2] + 64;
                float U = -.084368 * PIn_1[0] + .165632  * PIn_1[1] + .25      * PIn_1[2];
                float V =  .25     * PIn_2[0] - 0.209344 * PIn_2[1] - 0.040656 * PIn_2[2];
                
                // TODO add noise to pixels
                POut[0] = Y                + 1.402    * V;
                POut[1] = Y - 0.344136 * U - 0.714136 * V;
                POut[2] = Y + 1.772    * U;
                POut[3] = PIn_0[3];
            }

            std::fill(outframe + row, outframe + row + shift, 0);
        }

        for (unsigned int j = m_roll_position; j < roll_end; ++j)
        {
            for (unsigned int i = 0; i < m_width; ++i)
            {
                const size_t pIn = size_t(j) * m_width + i;
                const size_t pOut = size_t(m_roll_position) * m_width + i;

                outframe[pIn] = outframe[pOut];
            }
        }

        // TODO make strokes realistic and probabilize
        for (unsigned int j = 0; j < m_scratch_count; ++j)
        {
            unsigned int scratch_length = m_scratch_length(m_prng);

            size_t scratch_end = m_scratch_position(m_prng);
            size_t scratch_begin = std::max<size_t>(scratch_length, scratch_end) - scratch_length;

            for (size_t i = scratch_begin; i < scratch_end; ++i)
            {
                uint8_t *P = reinterpret_cast<uint8_t*>(outframe + i);

                P[0] = P[1] = P[2] = 255;
            }
        }

        // TODO lowpass with DCT

        m_roll_position = (m_roll_position + m_roll_step) % m_height;
    }
};

extern "C" {
#include <frei0r.h>

int f0r_init()
{
    return 0;
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
    return new vhs(width, height);
}

void f0r_get_plugin_info(f0r_plugin_info_t *info)
{
    info->name = "vhs";
    info->author = "xsbee";
    info->explanation = "VHS tape-like effect";
    info->major_version = 1;
    info->minor_version = 0;
    info->frei0r_version = FREI0R_MAJOR_VERSION;

    info->color_model = F0R_COLOR_MODEL_RGBA8888;
    info->plugin_type = F0R_PLUGIN_TYPE_FILTER;

    info->num_params = 6;
}

void f0r_get_param_info	(f0r_param_info_t *info, int param_index)
{
    switch (param_index)
    {
    case 0:
        info->name = "Roll Step";
        info->explanation = "Horizontal rolling blur step";
        info->type = F0R_PARAM_DOUBLE;
    break;
    case 1:
        info->name = "Roll Length";
        info->explanation = "Horizontal rolling blur length";
        info->type = F0R_PARAM_DOUBLE;
    break;
    case 2:
        info->name = "Chroma Shift";
        info->explanation = "Shift of chroma planes";
        info->type = F0R_PARAM_DOUBLE;
    break;
    case 3:
        info->name = "Scratch Density";
        info->explanation = "Density of white scratches";
        info->type = F0R_PARAM_DOUBLE;
    break;
    case 4:
        info->name = "Scratch Length";
        info->explanation = "Length of white scratches";
        info->type = F0R_PARAM_DOUBLE;
    break;
    case 5:
        info->name = "Right Shift";
        info->explanation = "Shift of rows towards right";
        info->type = F0R_PARAM_POSITION;
    break;
    }
}

void f0r_get_param_value(
    f0r_instance_t instance,
    f0r_param_t param,
    int param_index 
)
{
    struct vhs *context = static_cast<struct vhs*>(instance);

    switch (param_index)
    {
    case 0:
        *static_cast<double*>(param) = static_cast<double>(context->m_roll_step) / context->m_height;
    break;
    case 1:
        *static_cast<double*>(param) = static_cast<double>(context->m_roll_size) / context->m_height;
    break;
    case 2:
        *static_cast<double*>(param) = static_cast<double>(context->m_chroma_shift) / context->m_width;
    break;
    case 3:
        *static_cast<double*>(param) = static_cast<double>(context->m_scratch_count) / 1024;
    break;
    case 4:
        *static_cast<f0r_param_position_t*>(param) = {
            static_cast<double>(context->m_scratch_length.a()) / context->m_width,
            static_cast<double>(context->m_scratch_length.b()) / context->m_width
        };
    break;
    case 5:
        *static_cast<f0r_param_position_t*>(param) = {
            static_cast<double>(context->m_right_shift.a()) / context->m_width,
            static_cast<double>(context->m_right_shift.b()) / context->m_width
        };
    break;
    }
}

void f0r_set_param_value(
    f0r_instance_t instance,
    f0r_param_t param,
    int param_index 
)
{
    struct vhs *context = static_cast<struct vhs*>(instance);

    switch (param_index)
    {
    case 0:
        context->m_roll_step = *static_cast<double*>(param) * context->m_height;
    break;
    case 1:
        context->m_roll_size = *static_cast<double*>(param) * context->m_height;
    break;
    case 2:
        context->m_chroma_shift = *static_cast<double*>(param) * context->m_width;
    break;
    case 3:
        context->m_scratch_count = *static_cast<double*>(param) * 1024;
    break;
    case 4:
        auto scratch_length = static_cast<f0r_param_position_t*>(param);

        context->m_scratch_length.param(std::uniform_int_distribution<unsigned int>::param_type
        (
            scratch_length->x * context->m_width,
            scratch_length->y * context->m_width
        ));
    break;
    case 5:
        auto right_shift = static_cast<f0r_param_position_t*>(param);

        context->m_right_shift.param(std::uniform_int_distribution<unsigned int>::param_type
        (
            right_shift->x * context->m_width,
            right_shift->y * context->m_width
        ));
    }
}

void f0r_update	(
    f0r_instance_t instance,
    double,
    const uint32_t *inframe,
    uint32_t *outframe 
)
{
    static_cast<vhs*>(instance)->process(inframe, outframe);
}

void f0r_deinit() {}

void f0r_destruct (f0r_instance_t instance)	
{
    delete static_cast<vhs*>(instance);
}
}