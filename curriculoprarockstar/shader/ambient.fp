#version 130

in vec3 aCor; //recebe aCor do vp
uniform vec3 vertexColor;
uniform vec3 ambientLight;
out vec3 color;




void main(){
  color = vertexColor * ambientLight;



}