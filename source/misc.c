/*
 * This file is part of vitaGL
 * Copyright 2017, 2018, 2019, 2020 Rinnegatamante
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * misc.c:
 * Implementation for miscellaneous functions
 */

#include "shared.h"
GLboolean fast_texture_compression = GL_FALSE; // Hints for texture compression

static void update_fogging_state() {
#if defined(HAVE_SHARK) && defined(HAVE_SHARK_FFP)
	ffp_dirty_frag = GL_TRUE;
#endif
	if (fogging) {
		switch (fog_mode) {
		case GL_LINEAR:
			internal_fog_mode = LINEAR;
			break;
		case GL_EXP:
			internal_fog_mode = EXP;
			break;
		default:
			internal_fog_mode = EXP2;
			break;
		}
	} else
		internal_fog_mode = DISABLED;
}

static void update_polygon_offset() {
	switch (polygon_mode_front) {
	case SCE_GXM_POLYGON_MODE_TRIANGLE_LINE:
		if (pol_offset_line)
			sceGxmSetFrontDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else
			sceGxmSetFrontDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_POINT:
		if (pol_offset_point)
			sceGxmSetFrontDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else
			sceGxmSetFrontDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_FILL:
		if (pol_offset_fill)
			sceGxmSetFrontDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else
			sceGxmSetFrontDepthBias(gxm_context, 0, 0);
		break;
	}
	switch (polygon_mode_back) {
	case SCE_GXM_POLYGON_MODE_TRIANGLE_LINE:
		if (pol_offset_line)
			sceGxmSetBackDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else
			sceGxmSetBackDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_POINT:
		if (pol_offset_point)
			sceGxmSetBackDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else
			sceGxmSetBackDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_FILL:
		if (pol_offset_fill)
			sceGxmSetBackDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else
			sceGxmSetBackDepthBias(gxm_context, 0, 0);
		break;
	}
}

void change_cull_mode() {
	// Setting proper cull mode in sceGxm depending to current openGL machine state
	if (cull_face_state) {
#ifdef HAVE_UNFLIPPED_FBOS
		if ((gl_front_face == GL_CW) && (gl_cull_mode == GL_BACK))
			sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CCW);
		else if ((gl_front_face == GL_CCW) && (gl_cull_mode == GL_BACK))
			sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CW);
		else if ((gl_front_face == GL_CCW) && (gl_cull_mode == GL_FRONT))
			sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CCW);
		else if ((gl_front_face == GL_CW) && (gl_cull_mode == GL_FRONT))
			sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CW);
#else
		if ((gl_front_face == GL_CW) && (gl_cull_mode == GL_BACK))
			sceGxmSetCullMode(gxm_context, is_rendering_display ? SCE_GXM_CULL_CCW : SCE_GXM_CULL_CW);
		else if ((gl_front_face == GL_CCW) && (gl_cull_mode == GL_BACK))
			sceGxmSetCullMode(gxm_context, is_rendering_display ? SCE_GXM_CULL_CW : SCE_GXM_CULL_CCW);
		else if ((gl_front_face == GL_CCW) && (gl_cull_mode == GL_FRONT))
			sceGxmSetCullMode(gxm_context, is_rendering_display ? SCE_GXM_CULL_CCW : SCE_GXM_CULL_CW);
		else if ((gl_front_face == GL_CW) && (gl_cull_mode == GL_FRONT))
			sceGxmSetCullMode(gxm_context, is_rendering_display ? SCE_GXM_CULL_CW : SCE_GXM_CULL_CCW);
#endif
		else if (gl_cull_mode == GL_FRONT_AND_BACK)
			no_polygons_mode = GL_TRUE;
	} else
		sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_NONE);
}

/*
 * ------------------------------
 * - IMPLEMENTATION STARTS HERE -
 * ------------------------------
 */

void glPolygonMode(GLenum face, GLenum mode) {
	SceGxmPolygonMode new_mode;
	switch (mode) {
	case GL_POINT:
		new_mode = SCE_GXM_POLYGON_MODE_TRIANGLE_POINT;
		break;
	case GL_LINE:
		new_mode = SCE_GXM_POLYGON_MODE_TRIANGLE_LINE;
		break;
	case GL_FILL:
		new_mode = SCE_GXM_POLYGON_MODE_TRIANGLE_FILL;
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
	switch (face) {
	case GL_FRONT:
		polygon_mode_front = new_mode;
		gl_polygon_mode_front = mode;
		sceGxmSetFrontPolygonMode(gxm_context, new_mode);
		break;
	case GL_BACK:
		polygon_mode_back = new_mode;
		gl_polygon_mode_back = mode;
		sceGxmSetBackPolygonMode(gxm_context, new_mode);
		break;
	case GL_FRONT_AND_BACK:
		polygon_mode_front = polygon_mode_back = new_mode;
		gl_polygon_mode_front = gl_polygon_mode_back = mode;
		sceGxmSetFrontPolygonMode(gxm_context, new_mode);
		sceGxmSetBackPolygonMode(gxm_context, new_mode);
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		return;
	}
	update_polygon_offset();
}

void glPolygonOffset(GLfloat factor, GLfloat units) {
	pol_factor = factor;
	pol_units = units;
	update_polygon_offset();
}

void glCullFace(GLenum mode) {
	gl_cull_mode = mode;
	if (cull_face_state)
		change_cull_mode();
}

void glFrontFace(GLenum mode) {
	gl_front_face = mode;
	if (cull_face_state)
		change_cull_mode();
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
#ifndef SKIP_ERROR_HANDLING
	if ((width < 0) || (height < 0)) {
		SET_GL_ERROR(GL_INVALID_VALUE)
	}
#endif
	
	x_scale = width >> 1;
	x_port = x + x_scale;
	y_scale = -(height >> 1);
	y_port = (is_rendering_display ? DISPLAY_HEIGHT : in_use_framebuffer->height) - y + y_scale;
#ifndef HAVE_UNFLIPPED_FBOS
	if (!is_rendering_display) y_scale = -y_scale;
#endif
	setViewport(gxm_context, x_port, x_scale, y_port, y_scale, z_port, z_scale);
	gl_viewport.x = x;
	gl_viewport.y = y;
	gl_viewport.w = width;
	gl_viewport.h = height;
}

void glDepthRange(GLdouble nearVal, GLdouble farVal) {
	z_port = (farVal + nearVal) / 2.0f;
	z_scale = (farVal - nearVal) / 2.0f;
	setViewport(gxm_context, x_port, x_scale, y_port, y_scale, z_port, z_scale);
}

void glDepthRangef(GLfloat nearVal, GLfloat farVal) {
	z_port = (farVal + nearVal) / 2.0f;
	z_scale = (farVal - nearVal) / 2.0f;
	setViewport(gxm_context, x_port, x_scale, y_port, y_scale, z_port, z_scale);
}

void glEnable(GLenum cap) {
#ifndef SKIP_ERROR_HANDLING
	if (phase == MODEL_CREATION) {
		SET_GL_ERROR(GL_INVALID_OPERATION)
	}
#endif
	switch (cap) {
	case GL_DEPTH_TEST:
		depth_test_state = GL_TRUE;
		change_depth_func();
		break;
	case GL_STENCIL_TEST:
		stencil_test_state = GL_TRUE;
		change_stencil_settings();
		break;
	case GL_BLEND:
		if (!blend_state)
			change_blend_factor();
		blend_state = GL_TRUE;
		break;
	case GL_SCISSOR_TEST:
		scissor_test_state = GL_TRUE;
		sceneReset();
		update_scissor_test();
		break;
	case GL_CULL_FACE:
		cull_face_state = GL_TRUE;
		change_cull_mode();
		break;
	case GL_POLYGON_OFFSET_FILL:
		pol_offset_fill = GL_TRUE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_LINE:
		pol_offset_line = GL_TRUE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_POINT:
		pol_offset_point = GL_TRUE;
		update_polygon_offset();
		break;
	case GL_TEXTURE_2D:
		texture_units[server_texture_unit].enabled = GL_TRUE;
		break;
	case GL_ALPHA_TEST:
		alpha_test_state = GL_TRUE;
		update_alpha_test_settings();
		break;
	case GL_FOG:
		fogging = GL_TRUE;
		update_fogging_state();
		break;
	case GL_CLIP_PLANE0:
#if defined(HAVE_SHARK) && defined(HAVE_SHARK_FFP)
		ffp_dirty_vert = GL_TRUE;
#endif
		clip_plane0 = GL_TRUE;
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}

void glDisable(GLenum cap) {
#ifndef SKIP_ERROR_HANDLING
	if (phase == MODEL_CREATION) {
		SET_GL_ERROR(GL_INVALID_OPERATION)
	}
#endif
	switch (cap) {
	case GL_DEPTH_TEST:
		depth_test_state = GL_FALSE;
		change_depth_func();
		break;
	case GL_STENCIL_TEST:
		stencil_test_state = GL_FALSE;
		change_stencil_settings();
		break;
	case GL_BLEND:
		if (blend_state)
			change_blend_mask();
		blend_state = GL_FALSE;
		break;
	case GL_SCISSOR_TEST:
		scissor_test_state = GL_FALSE;
		sceneReset();
		update_scissor_test();
		break;
	case GL_CULL_FACE:
		cull_face_state = GL_FALSE;
		change_cull_mode();
		break;
	case GL_POLYGON_OFFSET_FILL:
		pol_offset_fill = GL_FALSE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_LINE:
		pol_offset_line = GL_FALSE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_POINT:
		pol_offset_point = GL_FALSE;
		update_polygon_offset();
		break;
	case GL_TEXTURE_2D:
		texture_units[server_texture_unit].enabled = GL_FALSE;
		break;
	case GL_ALPHA_TEST:
		alpha_test_state = GL_FALSE;
		update_alpha_test_settings();
		break;
	case GL_FOG:
		fogging = GL_FALSE;
		update_fogging_state();
		break;
	case GL_CLIP_PLANE0:
#if defined(HAVE_SHARK) && defined(HAVE_SHARK_FFP)
		ffp_dirty_vert = GL_TRUE;
#endif
		clip_plane0 = GL_FALSE;
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}

void glClear(GLbitfield mask) {
	sceneReset();
	
	// Invalidating viewport and culling
	invalidate_viewport();
	sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_NONE);
	
	void *fbuffer, *vbuffer;
	
	GLenum orig_depth_test = depth_test_state;
	if ((mask & GL_COLOR_BUFFER_BIT) == GL_COLOR_BUFFER_BIT) {
		invalidate_depth_test();
		change_depth_write(SCE_GXM_DEPTH_WRITE_DISABLED);
		sceGxmSetFrontPolygonMode(gxm_context, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);
		sceGxmSetBackPolygonMode(gxm_context, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);
		sceGxmSetVertexProgram(gxm_context, clear_vertex_program_patched);
		sceGxmSetFragmentProgram(gxm_context, clear_fragment_program_patched);
		sceGxmReserveFragmentDefaultUniformBuffer(gxm_context, &fbuffer);
		sceGxmSetUniformDataF(fbuffer, clear_color, 0, 4, &clear_rgba_val.r);
		sceGxmReserveVertexDefaultUniformBuffer(gxm_context, &vbuffer);
		sceGxmSetUniformDataF(vbuffer, clear_position, 0, 4, &clear_vertices->x);
		sceGxmDraw(gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, depth_clear_indices, 4);
		validate_depth_test();
		change_depth_write(depth_mask_state ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
		sceGxmSetFrontPolygonMode(gxm_context, polygon_mode_front);
		sceGxmSetBackPolygonMode(gxm_context, polygon_mode_back);
	}
	if ((mask & GL_DEPTH_BUFFER_BIT) == GL_DEPTH_BUFFER_BIT) {
		invalidate_depth_test();
		change_depth_write(SCE_GXM_DEPTH_WRITE_ENABLED);
		sceGxmSetVertexProgram(gxm_context, clear_vertex_program_patched);
		sceGxmSetFragmentProgram(gxm_context, disable_color_buffer_fragment_program_patched);
		sceGxmReserveFragmentDefaultUniformBuffer(gxm_context, &fbuffer);
		float temp = depth_value;
		sceGxmSetUniformDataF(fbuffer, clear_depth, 0, 1, &temp);
		sceGxmReserveVertexDefaultUniformBuffer(gxm_context, &vbuffer);
		sceGxmSetUniformDataF(vbuffer, clear_position, 0, 4, &clear_vertices->x);
		sceGxmDraw(gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, depth_clear_indices, 4);
		validate_depth_test();
		change_depth_write(depth_mask_state ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
	}
	if ((mask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT) {
		invalidate_depth_test();
		change_depth_write(SCE_GXM_DEPTH_WRITE_DISABLED);
		sceGxmSetVertexProgram(gxm_context, clear_vertex_program_patched);
		sceGxmSetFragmentProgram(gxm_context, disable_color_buffer_fragment_program_patched);
		sceGxmSetFrontStencilFunc(gxm_context,
			SCE_GXM_STENCIL_FUNC_NEVER,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			0, stencil_value * 0xFF);
		sceGxmSetBackStencilFunc(gxm_context,
			SCE_GXM_STENCIL_FUNC_NEVER,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			0, stencil_value * 0xFF);
		sceGxmReserveFragmentDefaultUniformBuffer(gxm_context, &fbuffer);
		float temp = 1.0f;
		sceGxmSetUniformDataF(fbuffer, clear_depth, 0, 1, &temp);
		sceGxmReserveVertexDefaultUniformBuffer(gxm_context, &vbuffer);
		sceGxmSetUniformDataF(vbuffer, clear_position, 0, 4, &clear_vertices->x);
		sceGxmDraw(gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, depth_clear_indices, 4);
		validate_depth_test();
		change_depth_write(depth_mask_state ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
		change_stencil_settings();
	}
	
	// Restoring viewport and culling
	validate_viewport();
	change_cull_mode();
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	clear_rgba_val.r = red;
	clear_rgba_val.g = green;
	clear_rgba_val.b = blue;
	clear_rgba_val.a = alpha;
}

void glLineWidth(GLfloat width) {
#ifndef SKIP_ERROR_HANDLING
	// Error handling
	if (width <= 0) {
		SET_GL_ERROR(GL_INVALID_VALUE)
	}
#endif

	// Changing line and point width as requested
	sceGxmSetFrontPointLineWidth(gxm_context, width);
	sceGxmSetBackPointLineWidth(gxm_context, width);
}

void glPointSize(GLfloat size) {
#ifndef SKIP_ERROR_HANDLING
	// Error handling
	if (size <= 0) {
		SET_GL_ERROR(GL_INVALID_VALUE)
	}
#endif

	// Changing line and point width as requested
	sceGxmSetFrontPointLineWidth(gxm_context, size);
	sceGxmSetBackPointLineWidth(gxm_context, size);
}

void glFogf(GLenum pname, GLfloat param) {
	switch (pname) {
	case GL_FOG_MODE:
		fog_mode = param;
		update_fogging_state();
		break;
	case GL_FOG_DENSITY:
		fog_density = param;
		break;
	case GL_FOG_START:
		fog_near = param;
		break;
	case GL_FOG_END:
		fog_far = param;
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}

void glFogfv(GLenum pname, const GLfloat *params) {
	switch (pname) {
	case GL_FOG_MODE:
		fog_mode = params[0];
		update_fogging_state();
		break;
	case GL_FOG_DENSITY:
		fog_density = params[0];
		break;
	case GL_FOG_START:
		fog_near = params[0];
		break;
	case GL_FOG_END:
		fog_far = params[0];
		break;
	case GL_FOG_COLOR:
		memcpy_neon(&fog_color.r, params, sizeof(vector4f));
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}

void glFogi(GLenum pname, const GLint param) {
	switch (pname) {
	case GL_FOG_MODE:
		fog_mode = param;
		update_fogging_state();
		break;
	case GL_FOG_DENSITY:
		fog_density = param;
		break;
	case GL_FOG_START:
		fog_near = param;
		break;
	case GL_FOG_END:
		fog_far = param;
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}

void glClipPlane(GLenum plane, const GLdouble *equation) {
	switch (plane) {
	case GL_CLIP_PLANE0:
		clip_plane0_eq.x = equation[0];
		clip_plane0_eq.y = equation[1];
		clip_plane0_eq.z = equation[2];
		clip_plane0_eq.w = equation[3];
		matrix4x4 inverted, inverted_transposed;
		matrix4x4_invert(inverted, modelview_matrix);
		matrix4x4_transpose(inverted_transposed, inverted);
		vector4f temp;
		vector4f_matrix4x4_mult(&temp, inverted_transposed, &clip_plane0_eq);
		memcpy_neon(&clip_plane0_eq.x, &temp.x, sizeof(vector4f));
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}

void glHint(GLenum target, GLenum mode) {
	switch (target) {
	case GL_TEXTURE_COMPRESSION_HINT:
		switch (mode) {
		case GL_FASTEST:
			fast_texture_compression = GL_TRUE;
			break;
		default:
			fast_texture_compression = GL_FALSE;
			break;
		}
		break;
	default:
		SET_GL_ERROR(GL_INVALID_ENUM)
		break;
	}
}
