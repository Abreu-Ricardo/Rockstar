#version 130 

in vec3 aCor; //recebe aCor do vp

//in vec3 theNormal;
//in vec3 thePosition;
in vec3 diffuse;


//uniform vec3 vertexColor;
//uniform vec3 ambientLight;

//uniform vec3 lightPosition;

//uniform vec3 diffuseIntensity;


out vec3 color;





void main(){
  color = aCor * vec3(0.5,0.5,0.5);

  //vec3 lightVector = normalize(lightPosition - theNormal);
  //float brightness = dot(lightVector,theNormal);
  //color = vec4(brightness,brightness,brightness,1.0);

  //Difusa
  //vec3 lightVector = normalize(lightPosition - thePosition);
 // color =   diffuseIntensity * vertexColor * dot(lightVector, theNormal);
}