/* 
 * shared.h:
 * All functions/definitions that shouldn't be exposed to
 * end users but are used in multiple source files must be here
 */
 
#ifndef _SHARED_H_
#define _SHARED_H_
 
#include <stdlib.h>
#include <stdio.h>
#include <vitasdk.h> 
#include "vitaGL.h"
#include "utils/math_utils.h"
#include "utils/gpu_utils.h"
#include "state.h"

// Internal constants
#define TEXTURES_NUM          1024 // Available textures per texture unit
#define MODELVIEW_STACK_DEPTH 32   // Depth of modelview matrix stack
#define GENERIC_STACK_DEPTH   2    // Depth of generic matrix stack
#define DISPLAY_WIDTH         960  // Display width in pixels
#define DISPLAY_HEIGHT        544  // Display height in pixels
#define DISPLAY_STRIDE        1024 // Display stride in pixels
#define DISPLAY_BUFFER_COUNT  2    // Display buffers to use
#define GXM_TEX_MAX_SIZE      4096 // Maximum width/height in pixels per texture
#define BUFFERS_ADDR        0xA000 // Starting address for buffers indexing
#define BUFFERS_NUM           128  // Maximum number of allocatable buffers
#define MAX_CUSTOM_SHADERS    32   // Maximum number of linkable custom shaders
#define MAX_SHADER_PARAMS 16 // Maximum number of parameters per custom shader

// Internal stuffs
void* frag_uniforms = NULL;
void* vert_uniforms = NULL;

// Debugging tool
#ifdef ENABLE_LOG
void LOG(const char *format, ...) {
	__gnuc_va_list arg;
	int done;
	va_start(arg, format);
	char msg[512];
	done = vsprintf(msg, format, arg);
	va_end(arg);
	int i;
	sprintf(msg, "%s\n", msg);
	FILE* log = fopen("ux0:/data/vitaGL.log", "a+");
	if (log != NULL) {
		fwrite(msg, 1, strlen(msg), log);
		fclose(log);
	}
}
#endif

// Depending on SDK, that could be or not defined
#ifndef max
#  define max(a,b) ((a) > (b) ? (a) : (b))
#endif

SceGxmContext *gxm_context; // sceGxm context instance
GLenum error = GL_NO_ERROR; // Error returned by glGetError
SceGxmShaderPatcher *gxm_shader_patcher; // sceGxmShaderPatcher shader patcher instance

matrix4x4 mvp_matrix; // ModelViewProjection Matrix
matrix4x4 projection_matrix; // Projection Matrix
matrix4x4 modelview_matrix; // ModelView Matrix

GLuint cur_program = 0; // Current in-use custom program (0 = No custom program)

/* custom_shaders.c */
void resetCustomShaders(void); // Resets custom shaders
void changeCustomShadersBlend(SceGxmBlendInfo *blend_info); // Change SceGxmBlendInfo value to all custom shaders
void reloadCustomShader(void); // Reloads in use custom shader inside sceGxm
void _vglDrawObjects_CustomShadersIMPL(GLenum mode, GLsizei count, GLboolean implicit_wvp); // vglDrawObjects implementation for rendering with custom shaders

#endif