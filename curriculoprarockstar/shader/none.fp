#version 130

//in vec3 aCor; //recebe aCor do vp

out vec3 color;
uniform vec3 vertexColor;



void main(){
  color = vertexColor;//vec4(aCor, 1);



}