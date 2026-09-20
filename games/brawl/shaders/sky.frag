#version 330 core
in vec2 uv;out vec4 frag;
uniform sampler2D noiseTex;uniform vec2 resolution;uniform float time,fov;uniform int chapter;uniform vec3 eye,target,up;uniform float seed;
float noise3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 u=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(u+.5)/256.).r,texture(noiseTex,(u+vec2(37,17)+.5)/256.).r,f.z);}
float fbm(vec3 p){float v=0,a=.53;for(int i=0;i<5;i++){v+=noise3(p)*a;p=p*2.02+vec3(17,9,21);a*=.5;}return v;}
void main(){
 vec2 q=(uv-.5)*2.;q.x*=resolution.x/resolution.y;
 vec3 F=normalize(target-eye),R=normalize(cross(F,up)),U=cross(R,F);
 vec3 ray=normalize(F+R*q.x*tan(fov*.5)+U*q.y*tan(fov*.5));
 bool day=chapter==1||chapter==3;bool snow=chapter==2;
 vec3 horizon=day?vec3(.28,.48,.53):snow?vec3(.21,.32,.46):vec3(.38,.20,.17);
 vec3 zenith=day?vec3(.045,.21,.42):snow?vec3(.032,.10,.23):vec3(.07,.10,.22);
 vec3 col=mix(horizon,zenith,pow(clamp(ray.y,0.,1.),.55));
 vec3 sun=normalize(vec3(-.50,.82,.65));float alignment=max(dot(ray,sun),0.);
 col+=vec3(1.,.80,.49)*pow(alignment,32.)*(day?.30:.1);
 col+=vec3(3.,2.5,1.7)*smoothstep(.9991,.9996,alignment);
 if(ray.y>.015){
  vec2 cloudUV=ray.xz/(ray.y+.22)*1.4+vec2(time*.0018,0);
  float cloud=fbm(vec3(cloudUV,4.3)),detail=fbm(vec3(cloudUV*2.9,12.));
  float density=smoothstep(.48,.72,cloud+detail*.13)*smoothstep(.015,.13,ray.y);
  vec3 cloudColor=day?mix(vec3(.52,.63,.59),vec3(.97,.98,.83),cloud):mix(vec3(.17,.22,.35),vec3(.49,.46,.51),cloud);
  col=mix(col,cloudColor,density*.87);
 }
 // A pale companion moon echoes the installed Bliss wallpaper without overpowering the sky.
 vec3 moon=normalize(vec3(.45,.35,-.9));float md=dot(ray,moon);
 if(md>.986){float disk=smoothstep(.986,.987,md);float grain=fbm(ray*73.);col=mix(col,vec3(.59,.73,.76)*(.72+grain*.34),disk*(day?.24:.6));}
 if(snow){float curtain=pow(.5+.5*sin(ray.y*18.+fbm(ray*3.+vec3(time*.004,0,0))*9.),7.);col+=vec3(.026,.23,.14)*curtain*smoothstep(.05,.7,ray.y);}
 if(ray.y<0)col=mix(col,day?vec3(.16,.26,.21):vec3(.09,.13,.19),smoothstep(0.,.45,-ray.y));
 frag=vec4(col,1);
}
