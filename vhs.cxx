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