#include <random>
#include <algorithm>

#include <frei0r.h>

struct vhs
{
    unsigned int m_width;
    unsigned int m_height;
    vhs(unsigned int width, unsigned int height)
        : m_width(width), m_height(height),
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
}

void f0r_get_param_value(
    f0r_instance_t instance,
    f0r_param_t param,
    int param_index 
)
{
}

void f0r_set_param_value(
    f0r_instance_t instance,
    f0r_param_t param,
    int param_index 
)
{
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