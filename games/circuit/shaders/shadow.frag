#version 330 core
in vec2 uv;in vec4 mat;
uniform sampler2D hullTex;
void main(){
 if(mat.z>15.5&&mat.z<17.0){float row=floor((mat.z-16.0)*10.0+.25);if(texture(hullTex,vec2(uv.x,(row+1.-uv.y)/8.)).a<.08)discard;}
}
