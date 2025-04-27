#Name XXX

#Properties

color prop1;
...

#EndProperties

#BeginBlock A
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) in X XXX;
layout(location = 1) in X XXX;
layout(location = 2) in X XXX;

layout(location = 0) out X XXX;
void main()
{
	...
}
#EndBlock

#BeginBlock B
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out X XXX;

layout(location = 1) in X XXX;

void main()
{
	...
}

#EndBlock

#BeginBlock C
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out X XXX;

layout(location = 1) in X XXX;

void main()
{
	...
}

#EndBlock


##Pass 1
Blend Add SrcAlpha OneMinusSrcAlpha
DepthWrite On
DepthTest LEqual
Cull Back
Vertex A
Fragment B
##EndPass

##Pass 2
Blend None
DepthWrite Off
DepthTest GEqual
Cull Front
Vertex A
Fragment C
##EndPass