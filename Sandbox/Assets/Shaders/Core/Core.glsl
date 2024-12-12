layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 2) uniform TIME
{
	vec4 GE_TIME;
};
layout(std140, binding = 3) uniform MODEL
{
	mat4 GE_MATRIX_M;
};
layout(std140, binding = 4) uniform LIGHT
{
	vec4 GE_MAIN_LIGHT_DIRECTION;
	vec4 GE_MAIN_LIGHT_COLOR;
};
layout(std140, binding = 5) uniform SCREEN
{
	vec4 GE_SCREEN_SIZE;	
};
float FloatToSrgb(float value) {
    const float inv_12_92 = 0.0773993808;
    return value <= 0.04045
       ? value * inv_12_92 
       : pow((value + 0.055) / 1.055, 2.4);
}
vec3 Vec3ToSrgb(vec3 value) {
    return vec3(FloatToSrgb(value.x), FloatToSrgb(value.y), FloatToSrgb(value.z));
}
vec2 TransformUV(vec2 uv)
{
#if GE_ATTACHMENT_UV_STARTS_AT_TOP
	uv.y = 1 - uv.y;
#endif
	return uv;
}