#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <frei0r.h>

struct ilac0r
{
    unsigned int width;
    unsigned int height;

    uint32_t *framebuffer;
    size_t framebuffersize;

    bool oddframe;
};

int f0r_init ()
{
    return 0;
}

f0r_instance_t f0r_construct (unsigned int width, unsigned int height)
{
    struct ilac0r *instance = malloc (sizeof (struct ilac0r));

    if (!instance)
        return NULL;

    instance->width = (width / 2) * 2;
    instance->height = (height / 2) * 2;

    instance->framebuffersize = sizeof (uint32_t) * width * height;
    instance->framebuffer = calloc (instance->framebuffersize, 1);

    instance->oddframe = false;

    return instance;
}

void f0r_get_plugin_info (f0r_plugin_info_t *info)
{
    info->name = "ilac0r";
    info->author = "xsbee";
    info->explanation = "Horizontal and vetical interlacer";
    info->major_version = 1;
    info->minor_version = 0;
    info->frei0r_version = FREI0R_MAJOR_VERSION;

    info->color_model = F0R_COLOR_MODEL_PACKED32;
    info->plugin_type = F0R_PLUGIN_TYPE_FILTER;
    info->num_params = 0;
}

void f0r_get_param_info	(f0r_param_info_t *info, int param_index) {}
void f0r_get_param_value (f0r_instance_t instance, f0r_param_t param, int param_index) {}
void f0r_set_param_value (f0r_instance_t instance, f0r_param_t param, int param_index) {}

void f0r_update	(f0r_instance_t instance, double time, const uint32_t *inframe, uint32_t *outframe)
{
    struct ilac0r *context = instance;

    for (unsigned int j = 0; j < context->height; j += 2)
    {
        for (unsigned int i = 0; i < context->width; i += 2)
        {
            size_t p0 =  j    * context->width + i + context->oddframe,
                   p1 = (j+1) * context->width + i + context->oddframe;

            context->framebuffer[p0] = inframe[p0];
            context->framebuffer[p1] = inframe[p1];
        }
    }
    
    memcpy (outframe, context->framebuffer, context->framebuffersize);

    context->oddframe = !context->oddframe;
}

void f0r_deinit() {}
void f0r_destruct (f0r_instance_t instance)	
{
    struct ilac0r *context = instance;

    free (context->framebuffer);
    free (context);
}
