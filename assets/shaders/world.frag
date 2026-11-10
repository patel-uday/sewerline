#version 330 core
in vec2 vUV;in vec3 vNormal;in float vDepth;out vec4 FragColor;
uniform sampler2D uTex;uniform vec4 uTint;uniform vec3 uFog;uniform int uMode;
void main(){vec4 tex=texture(uTex,vUV);if(tex.a<.1)discard;float facet=uMode==0?(.42+.45*max(0.0,dot(normalize(vNormal),normalize(vec3(.3,.8,.2))))):1.0;vec4 c=tex*uTint*vec4(vec3(facet),1);float fog=clamp((vDepth-9.0)/35.0,0.0,.88);FragColor=vec4(mix(c.rgb,uFog,fog),c.a);}
