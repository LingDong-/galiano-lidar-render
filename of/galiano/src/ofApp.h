#pragma once

#include "ofMain.h"

#define MESH_T ofVboMesh

#define MGRD_SKIP (4)


class DistanceTransform {public:
  // ported from https://github.com/parmanoir/Meijster-distance
  uint16_t* dt;
  uint16_t* s;
  uint16_t* t;
  uint16_t* g;
  
  void setup(int m, int n){
    dt= (uint16_t*)malloc(m*n*sizeof(uint16_t));
    g = (uint16_t*)malloc(m*n*sizeof(uint16_t));
    s = (uint16_t*)malloc(m*sizeof(uint16_t));
    t = (uint16_t*)malloc(m*sizeof(uint16_t));
  }
  
  void destroy(){
    free(dt);
    free(g);
    free(s);
    free(t);
  }
  
  // Euclidean
  int EDT_f(int x, int i, int g_i) {
    return (x - i) * (x - i) + g_i * g_i;
  }
  int EDT_Sep(int i, int u, int g_i, int g_u) {
    return (u * u - i * i + g_u * g_u - g_i * g_i) / (2 * (u - i));
  }
  //  Meijster distance
  void compute(uint8_t* b, int m, int n) {
    // First phase
    int infinity = m + n;
    
    for (int x = 0; x < m; x++) {
      if (!b[x + 0 * m])
        g[x + 0 * m] = 0;
      else
        g[x + 0 * m] = infinity;
      // Scan 1
      for (int y = 1; y < n; y++) {
        if (!b[x + y * m])
          g[x + y * m] = 0;
        else
          g[x + y * m] = 1 + g[x + (y - 1) * m];
      }
      // Scan 2
      for (int y = n - 2; y >= 0; y--) {
          if (g[x + (y + 1) * m] < g[x + y * m])
            g[x + y * m] = 1 + g[x + (y + 1) * m];
      }
    }
    
    // Second phase
    int q = 0;
    int w;
    for (int y = 0; y < n; y++) {
      q = 0;
      s[0] = 0;
      t[0] = 0;
      
      // Scan 3
      for (int u = 1; u < m; u++) {
        while (q >= 0 && EDT_f(t[q], s[q], g[s[q] + y * m]) > EDT_f(t[q], u, g[u + y * m]))
          q--;
        if (q < 0) {
          q = 0;
          s[0] = u;
        } else {
          w = 1 + EDT_Sep(s[q], u, g[s[q] + y * m], g[u + y * m]);
          if (w < m) {
            q++;
            s[q] = u;
            t[q] = w;
          }
        }
      }
      // Scan 4
      for (int u = m - 1; u >= 0; u--) {
        int d = EDT_f(u, s[q], g[s[q] + y * m]);
        
        d = sqrt(d);
        dt[u + y * m] = d;
        if (u == t[q])
          q--;
      }
    }
    return dt;
  }
};

class ofApp : public ofBaseApp{public:
//  ofEasyCam cam;
  ofCamera cam;
  
  class Chunk {public:
    MESH_T meshes[4];
    MESH_T sea;
    ofVec2f size;
    ofVec3f center;
    string _id;

    Chunk(std::string ID){
      _id = ID;
    }
    void load(){
      size = load_mesh(_id,meshes[0]);
      process();
    }
    void process(){
      int w,h;
      ofVec2f wh = size;
      for (int i = 0; i < 3; i++){
        w = wh.x;
        h = wh.y;
        wh = half_mesh(meshes[i],meshes[i+1],w,h);
      }
      w = wh.x;
      h = wh.y;
      float sx = 0;
      float sy = 0;
      int sn = 0;
      
      for (int i = 0; i < w*h; i++){
        ofVec3f v = meshes[3].getVertex(i);
        if (v.x != -INFINITY && v.y != -INFINITY){
          sx += v.x;
          sy += v.y;
          sn ++;
        }
      }
      center.x = sx/sn;
      center.y = sy/sn;
      center.z = 0;
      
      sea.clear();
      
      uint8_t bmp[w*h];
      
      for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
          bmp[i*w+j] = meshes[3].getVertex(i*w+j).z<1;
        }
      }
      DistanceTransform dt;
      dt.setup(w,h);
      dt.compute(bmp,w,h);

      for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
          ofVec3f v = meshes[3].getVertex(i*w+j);
          float x = v.x;
          float y = v.y;
          if (x == -INFINITY){
            for (int k = 0; k < h; k++){
              if (meshes[3].getVertex(i*w+(j+k)%w).x != -INFINITY){
                x = meshes[3].getVertex(i*w+(j+k)%w).x;
              }
            }
          }
          if (y == -INFINITY){
            for (int k = 0; k < h; k++){
              if (meshes[3].getVertex(((i+k)%h)*w+j).y != -INFINITY){
                y = meshes[3].getVertex(((i+k)%h)*w+j).y;
              }
            }
          }
          float z = 1;
          float f = ofMap(dt.dt[i*w+j],0,5,1,0,true);
          f = f*f;
          f *= 60;
          
          sea.addVertex(ofVec3f(x,y,z));
          sea.addColor(ofColor(0+f*0.2,30+f,60+f*0.5));
        }
      }
      dt.destroy();
      
      for (int y = 0; y<h-1; y++){
        for (int x=0; x<w-1; x++){
          sea.addIndex(x+y*w);
          sea.addIndex((x+1)+y*w);
          sea.addIndex(x+(y+1)*w);
          sea.addIndex((x+1)+y*w);
          sea.addIndex((x+1)+(y+1)*w);
          sea.addIndex(x+(y+1)*w);
        }
      }
      
    }
    void serialize(){
      dump_vec(meshes[0].getVertices(),ofToDataPath("vrt_"+_id+".bin"));
      dump_vec(meshes[0].getColors(),ofToDataPath("clr_"+_id+".bin"));
      dump_vec(meshes[0].getIndices(),ofToDataPath("ind_"+_id+".bin"));
      dump_obj(size,ofToDataPath("siz_"+_id+".bin"));
    }
    void deserialize(){
      load_vec(meshes[0].getVertices(),ofToDataPath("vrt_"+_id+".bin"));
      load_vec(meshes[0].getColors(),ofToDataPath("clr_"+_id+".bin"));
      load_vec(meshes[0].getIndices(),ofToDataPath("ind_"+_id+".bin"));
      load_obj(size,ofToDataPath("siz_"+_id+".bin"));
      process();
    }
    void smart_load(){
      cout << "loading " << _id << " ..." << endl;
      std::ifstream inFile(ofToDataPath("siz_"+_id+".bin"), std::ios::binary);
      if (!inFile){
        load();
        serialize();
      }else{
        inFile.close();
        deserialize();
      }
    }
  };
  
  template <typename T>
  static void dump_obj(const T& obj, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(&obj), sizeof(T));
    outFile.close();
  }
  template <typename T>
  static void load_obj(T& obj, const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    inFile.read(reinterpret_cast<char*>(&obj), sizeof(T));
    inFile.close();
  }
  
  template <typename T>
  static void dump_vec(const std::vector<T>& vec, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    size_t size = vec.size();
    outFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
    for (const auto& obj : vec) {
      outFile.write(reinterpret_cast<const char*>(&obj), sizeof(T));
    }
    outFile.close();
  }
  template <typename T>
  static void load_vec(std::vector<T>& vec, const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    size_t size;
    inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
    vec.resize(size);
    for (auto& obj : vec) {
      inFile.read(reinterpret_cast<char*>(&obj), sizeof(T));
    }
    inFile.close();
  }
  
  vector<Chunk> chunks;
  
  static ofVec2f half_mesh(MESH_T& mesh, MESH_T& nesh, int w, int h){
    int nw = (w+1)/2;
    int nh = (h+1)/2;
    
    int64_t mv = mesh.getNumVertices()/2;
    
    for (int i = 0; i < h; i+=2){
      for (int j = 0; j < w; j+=2){
        nesh.addVertex(mesh.getVertex(i*w+j));
        nesh.addColor(mesh.getColor(i*w+j));
      }
    }
    
    for (int y = 0; y<nh-1; y++){
      for (int x=0; x<nw-1; x++){
        nesh.addIndex(x+y*nw);
        nesh.addIndex((x+1)+y*nw);
        nesh.addIndex(x+(y+1)*nw);
        nesh.addIndex((x+1)+y*nw);
        nesh.addIndex((x+1)+(y+1)*nw);
        nesh.addIndex(x+(y+1)*nw);
      }
    }
    int64_t nv = nesh.getNumVertices();
  
    for (int i = 0; i < h; i+=2){
      for (int j = 0; j < w; j+=2){
        nesh.addVertex(mesh.getVertex(mv+i*w+j));
        nesh.addColor(mesh.getColor(mv+i*w+j));
      }
    }
    for (int y = 0; y<nh-1; y++){
      for (int x=0; x<nw-1; x++){
        nesh.addIndex(nv+x+y*nw);
        nesh.addIndex(nv+(x+1)+y*nw);
        nesh.addIndex(nv+x+(y+1)*nw);
        nesh.addIndex(nv+(x+1)+y*nw);
        nesh.addIndex(nv+(x+1)+(y+1)*nw);
        nesh.addIndex(nv+x+(y+1)*nw);
      }
    }
    return ofVec2f(nw,nh);
  }
  
  static ofVec2f load_mesh(std::string ID, MESH_T& mesh){

    int MGRD_M, MGRD_N;
    
    FILE* fp = fopen(ofToDataPath("gnd_" +ID+ ".npy").c_str(),"rb");
    fseek(fp,0x3D,SEEK_SET);
    fscanf(fp,"%d, %d,",&MGRD_M,&MGRD_N);
    cout << MGRD_M << "x" << MGRD_N << endl;
    
    int w = (MGRD_N+MGRD_SKIP-1)/MGRD_SKIP;
    int h = (MGRD_M+MGRD_SKIP-1)/MGRD_SKIP;
    
    
    for (int i = 0; i < MGRD_M; i+=MGRD_SKIP){
      for (int j = 0; j < MGRD_N; j+=MGRD_SKIP){
        fseek(fp,128+(i*MGRD_N+j)*12,SEEK_SET);
        float x,y,z;
        fread(&x,4,1,fp);
        fread(&y,4,1,fp);
        fread(&z,4,1,fp);
        mesh.addVertex(ofVec3f(x,y,z));
      }
    }
    
    for (int y = 0; y<h; y++){
      for (int x=0; x<w; x++){
        if (x == w-1 || y==h-1){
          mesh.addColor(ofColor(200,150,100));
          continue;
        }
        ofVec3f v0 = mesh.getVertex(y*w+x);
        ofVec3f v1 = mesh.getVertex(y*w+x+1);
        ofVec3f v2 = mesh.getVertex((y+1)*w+x);
        ofVec3f p = (v1-v0).normalize();
        ofVec3f q = (v2-v0).normalize();
        ofVec3f n = p.cross(q);
//        cout << n << endl;
        if (isnan(n.x) || isnan(n.y) || isnan(n.z)){
          mesh.addColor(ofColor(200,150,100));
          continue;
        }
        ofVec3f l = ofVec3f(1,2,3).normalize();
        float ndl = fabs(n.dot(l));
        
        mesh.addColor(ofColor(200*ndl,150*ndl,100*ndl));
      }
    }
    
    for (int y = 0; y<h-1; y++){
      for (int x=0; x<w-1; x++){
        mesh.addIndex(x+y*w);
        mesh.addIndex((x+1)+y*w);
        mesh.addIndex(x+(y+1)*w);

        mesh.addIndex((x+1)+y*w);
        mesh.addIndex((x+1)+(y+1)*w);
        mesh.addIndex(x+(y+1)*w);
      }
    }
    int64_t nv = mesh.getNumVertices();
    
    fp = fopen(ofToDataPath("veg_" +ID+ ".npy").c_str(),"rb");
    for (int i = 0; i < MGRD_M; i+=MGRD_SKIP){
      for (int j = 0; j < MGRD_N; j+=MGRD_SKIP){
        fseek(fp,128+(i*MGRD_N+j)*12,SEEK_SET);
        float x,y,z;
        fread(&x,4,1,fp);
        fread(&y,4,1,fp);
        fread(&z,4,1,fp);
        mesh.addVertex(ofVec3f(x,y,z));
        
      }
    }
    
    for (int y = 0; y<h; y++){
      for (int x=0; x<w; x++){

        ofVec3f v0 = mesh.getVertex(y*w+x);
        ofVec3f v1 = mesh.getVertex(nv+y*w+x);
//        float c = ofMap(v1.z-v0.z,0,60,100,0);
        float d =v1.z-v0.z;
        if (d < -0.5){
          d = -0.5;
        }
        if (v0.z == -INFINITY){
          d = 0;
        }
  
        float r,g,b;
        if (d < 1){
          r = ofMap(d,-0.5,1,200,100);
          g = ofMap(d,-0.5,1,150,200);
          b = ofMap(d,-0.5,1,100,0);
        }else if (d < 4){
          r = ofMap(d,1,4,100,0);
          g = ofMap(d,1,4,200,0);
          b = ofMap(d,1,4,0,0);
        }else{
          r = ofMap(d,4,100,0,0);
          g = ofMap(d,4,100,0,255);
          b = ofMap(d,4,100,0,0);
        }
        mesh.addColor(ofColor(ofClamp(r,0,255),ofClamp(g,0,255),ofClamp(b,0,255)));
      }
    }

    for (int y = 0; y<h-1; y++){
      for (int x=0; x<w-1; x++){
        mesh.addIndex(nv+x+y*w);
        mesh.addIndex(nv+(x+1)+y*w);
        mesh.addIndex(nv+x+(y+1)*w);
        mesh.addIndex(nv+(x+1)+y*w);
        mesh.addIndex(nv+(x+1)+(y+1)*w);
        mesh.addIndex(nv+x+(y+1)*w);
      }
    }
    
    return ofVec2f(w,h);
  }
  
  void setup(){
    
    cam.setFarClip(100000);
//
//    chunks.push_back(Chunk("000"));
//    chunks.push_back(Chunk("001"));
//    chunks.push_back(Chunk("002"));
//    chunks.push_back(Chunk("003"));
//    chunks.push_back(Chunk("004"));
//    chunks.push_back(Chunk("005"));
//
    
    chunks.push_back(Chunk("092b083444"));
    chunks.push_back(Chunk("092b084312"));
    chunks.push_back(Chunk("092b084313"));
    chunks.push_back(Chunk("092b084314"));
    chunks.push_back(Chunk("092b084321"));
    chunks.push_back(Chunk("092b084322"));
    chunks.push_back(Chunk("092b084323"));
    chunks.push_back(Chunk("092b084324"));
    chunks.push_back(Chunk("092b084331"));
    chunks.push_back(Chunk("092b084332"));
    chunks.push_back(Chunk("092b084333"));
    chunks.push_back(Chunk("092b084334"));
    chunks.push_back(Chunk("092b084341"));
    chunks.push_back(Chunk("092b084342"));
    chunks.push_back(Chunk("092b084343"));
    chunks.push_back(Chunk("092b093143"));
    chunks.push_back(Chunk("092b093144"));
    chunks.push_back(Chunk("092b093214"));
    chunks.push_back(Chunk("092b093221"));
    chunks.push_back(Chunk("092b093222"));
    chunks.push_back(Chunk("092b093223"));
    chunks.push_back(Chunk("092b093224"));
    chunks.push_back(Chunk("092b093231"));
    chunks.push_back(Chunk("092b093232"));
    chunks.push_back(Chunk("092b093233"));
    chunks.push_back(Chunk("092b093234"));
    chunks.push_back(Chunk("092b093241"));
    chunks.push_back(Chunk("092b093242"));
    chunks.push_back(Chunk("092b093243"));
    chunks.push_back(Chunk("092b093314"));
    chunks.push_back(Chunk("092b093321"));
    chunks.push_back(Chunk("092b093322"));
    chunks.push_back(Chunk("092b093323"));
    chunks.push_back(Chunk("092b093324"));
    chunks.push_back(Chunk("092b093332"));
    chunks.push_back(Chunk("092b093333"));
    chunks.push_back(Chunk("092b093334"));
    chunks.push_back(Chunk("092b093341"));
    chunks.push_back(Chunk("092b093411"));
    chunks.push_back(Chunk("092b094111"));
    chunks.push_back(Chunk("092b094112"));
    chunks.push_back(Chunk("092b094113"));
    chunks.push_back(Chunk("092b094121"));
    chunks.push_back(Chunk("092g003111"));
    chunks.push_back(Chunk("092g003112"));
    
//    for (Chunk& chunk : chunks){
//      chunk.load();
//    }
    
//    for (Chunk& chunk : chunks){
//      chunk.load();
//      chunk.serialize();
//    }
    
//    for (Chunk& chunk : chunks) chunk.deserialize();
    
    for (Chunk& chunk : chunks) chunk.smart_load();

    cam.setPosition(ofVec3f(1000,1000,1000));
    cam.lookAt(ofVec3f(0,0,0),ofVec3f(0,0,1));
  }
  float rz=0;
  float rx=0;
  float vy=0;
  
  void update(){
//    int fr = ofGetFrameNum();
//
//    float x0 = -sin(fr*0.01)*600;
//    float y0 = fr*10;
//    float ang = M_PI*3/4-1.4;
//    float xe = x0*cos(ang)-y0*sin(ang);
//    float ye = x0*sin(ang)+y0*cos(ang);
//
//    int t = 300;
//
//    float x1 = -sin((fr+t)*0.01)*300;
//    float y1 = (fr+t)*10;
//    float xt = x1*cos(ang)-y1*sin(ang);
//    float yt = x1*sin(ang)+y1*cos(ang);
//
//    float ze = fmin(1500,fr*1.0);
//    float zt = fmin(1500,(fr+t)*1.0);
//
//    cam.setPosition(ofVec3f(1800+xe,-1800+ye,2000-ze));
//    cam.lookAt(     ofVec3f(1800+xt,-1800+yt,0),ofVec3f(0,0,1));
//    cam.tiltDeg(-20+fr*0.001);
//
//    return;
    if (ofGetKeyPressed('w')){
      vy += 1;
      rz -= 0.1;
    }
    if (ofGetKeyPressed('i')){
      vy += 1;
      rz += 0.1;
    }
    if (ofGetKeyPressed('s')){
      vy -= 1;
      rz += 0.1;
    }
    if (ofGetKeyPressed('k')){
      vy -= 1;
      rz -= 0.1;
    }
    if (ofGetKeyPressed('d')){
      rx -= 0.1;
    }
    if (ofGetKeyPressed('a')){
      rx += 0.1;
    }
    if (ofGetKeyPressed('j')){
      rx -= 0.1;
    }
    if (ofGetKeyPressed('l')){
      rx += 0.1;
    }
    
    cam.setPosition(cam.getPosition()+cam.getLookAtDir()*vy*0.8);
//    cam.panDeg(rz);
    cam.rotateDeg(rz*0.8,ofVec3f(0,0,1));
//    cam.rotateDeg(rx,ofVec3f(1,0,0));
    cam.tiltDeg(rx*0.8);
    
    if (cam.getPosition().z < 500){
      cam.setPosition(ofVec3f(cam.getPosition().x,cam.getPosition().y,500));
//      rx += 0.01;
    }
    vy *= 0.9;
    rz *= 0.9;
    rx *= 0.9;
    
//    cout << cam.getGlobalPosition() << endl;
  }
  void draw(){
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    ofSetBackgroundColor(130,160,190);
    
    ofEnableDepthTest();

    
    cam.begin();

    glEnable (GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    GLfloat fogColor[4] = {130.0/255,160.0/255,190.0/255};
    glFogfv(GL_FOG_COLOR, fogColor);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_DENSITY, 1.0f);
    glFogf(GL_FOG_START,800);
    glFogf(GL_FOG_END,5000);
    
    
    ofSetColor(0,30,60);
    float mw = 3280*0.565;
    float mh = 3280*0.425;
    ofDrawPlane(-mw*5,mh*4,mw*16,mh*18);

    ofSetColor(255);
    for (Chunk& chunk : chunks){
      ofVec3f p = cam.getGlobalPosition();
      ofVec3f q = chunk.center;
      float d = p.distance(q);
      int n = ofMap(d,800,5000,0,3.999,true);
//      cout << n << endl;
      chunk.sea.draw();
      chunk.meshes[n].draw();
//      chunk.meshes[3].draw();
    }

    
    ofDisableDepthTest();
    
    cam.end();
    
//    
//    std::ostringstream ss;
//    ss << std::setw(5) << std::setfill('0') << ofGetFrameNum();
//    ofSaveScreen("render/"+ss.str()+".png");
  }

  void keyPressed(int key){
    
  }
  void keyReleased(int key){
    
  }
  void mouseMoved(int x, int y ){
    
  }
  void mouseDragged(int x, int y, int button){
    
  }
  void mousePressed(int x, int y, int button){
    
  }
  void mouseReleased(int x, int y, int button){
    
  }
  void mouseEntered(int x, int y){
    
  }
  void mouseExited(int x, int y){
    
  }
  void windowResized(int w, int h){
    
  }
  void dragEvent(ofDragInfo dragInfo){
    
  }
  void gotMessage(ofMessage msg){
    
  }
		
};
