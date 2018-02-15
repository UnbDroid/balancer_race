import processing.net.*; 
import java.net.*;

Server agora_eu_sou_o_mestre; 
String dataIn;
String[] dataSplit = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};
String[] pastData = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};

float kalx, kaly, kalz;
float dt;

int i = 0;
int j = 0;
int k = 85;
int l = 170;

int jf = 0;
int kf = 0;
int lf = 0;

void setup() {
    // define tamanho da janela e modo de rendering 3D
    size(1000, 600, P3D);

    // torna o tamanho da janela redimencionavel.
    surface.setResizable(true);
    
    // setup lights and antialiasing
    lights();
    smooth();

    // Connect to the local machine at port 9001.
    // This example will not run if you haven't
    // previously started a server on this port.
    agora_eu_sou_o_mestre = new Server(this, 9003);
}

void draw() {
    // muda tipo de perspectiva para ortho
    ortho();
    
    // black background
    background(0);
    
    Client client = agora_eu_sou_o_mestre.available();
    if (client != null) {
        dataIn = client.readString();
        dataSplit = dataIn.split(";");
      
        kalx = radians(-Float.parseFloat(dataSplit[1])-90);   
        kaly = radians(-Float.parseFloat(dataSplit[0]));  
        kalz = radians(-Float.parseFloat(dataSplit[2])+180);
        dt = Float.parseFloat(dataSplit[3]);
        for (i = 0; i < 4; i++){
            pastData[i] = dataSplit[i];
        }
        
        agora_eu_sou_o_mestre.write("abacaxi");
        delay(50);
    }
    else
    {
        for(i = 0; i < 4; i++)
        {
            dataSplit[i] = pastData[i];
        }
        kalx = radians(-Float.parseFloat(dataSplit[1])-90);   
        kaly = radians(-Float.parseFloat(dataSplit[0]));  
        kalz = radians(-Float.parseFloat(dataSplit[2])+180);
        dt = Float.parseFloat(dataSplit[3]);
    }
  
    // mostra valores
    fill(255);
    text("Atitude", 130, 50);
    text("Roll", 30, 100);
    text("Pitch", 30, 150);
    text("Yall", 30, 200);
    text("dt", 30, 250);
    text(dataSplit[0], 130, 100);
    text(dataSplit[1], 130, 150);
    text(dataSplit[2], 130, 200);
    text(dataSplit[3], 130, 250);
    text("X", 30, 350);
    text("Y", 30, 400);
    text("Z", 30, 450);
    fill(255, 0, 255);
    text("Gyro", 130, 300);
    text("gX", 130, 350);
    text("gY", 130, 400);
    text("gZ", 130, 450);
    fill(255, 255, 0);
    text("Accel", 230, 300);
    text("aX", 230, 350);
    text("aY", 230, 400);
    text("aZ", 230, 450);
    fill(0, 255, 255);
    text("Magnet", 330, 300);
    text("mX", 330, 350);
    text("mY", 330, 400);
    text("mZ", 330, 450);
    
    // labels
    if (jf == 0) {
      j++;
      if (j == 255) jf = 1;
    }else{
      j--;
      if (j == 0) jf = 0;
    }
    if (kf == 0) {
      k++;
      if (k == 255) kf = 1;
    }else{
      k--;
      if (k == 0) kf = 0;
    }
    if (lf == 0) {
      l++;
      if (l == 255) lf = 1;
    }else{
      l--;
      if (l == 0) lf = 0;
    }
    textAlign(CENTER);
    textSize(16);
    fill(j, k, l);
    text("ATITUDE MATLAB", 6*width/8, height/8);
    
    // INICIO FILTRADO ##############################################################################################################
    pushMatrix();
    
    translate(6*width/8, height/3);
    criaCorpo(kalx, kaly, kalz, 255, 255, 255);
    
    popMatrix();
    // FIM FILTRADO
}

void criaCorpo(float x, float y, float z, int r, int g, int b)
{
    // rotaciona o objeto no espaço 3D
    rotate(x, 1, 0, 0);
    rotate(y, 0, 1, 0);
    rotate(z, 0, 0, 1);

    // draw main body
    fill(255, 0, 0, 200);
    box(10, 10, 120);
    pushMatrix();
    translate(0, 0, 65);
    box(90, 10, 10);
    fill(0, 255, 0, 200);
    translate(0, -10, -65);
    box(10, 10, 10);
    popMatrix();
    
    // draw top cilinder in the chosen color
    fill(r, g, b, 200);
    pushMatrix();
    translate(0, 0, -65);
    rotateX(PI/2);
    drawCylinder(40, 40, 5, 8);
    popMatrix();
    
    // draw weels
    fill(0, 0, 255, 200);
    pushMatrix();
    translate(50, 0, 65);
    rotateZ(PI/2);
    drawCylinder(25, 25, 5, 8);
    popMatrix();
    pushMatrix();
    translate(-45, 0, 65);
    rotateZ(PI/2);
    drawCylinder(25, 25, 5, 8);
    popMatrix();  
}

void drawCylinder(float topRadius, float bottomRadius, float tall, int sides) {
    float angle = 0;
    float angleIncrement = TWO_PI / sides;
    beginShape(QUAD_STRIP);
    for (int i = 0; i < sides + 1; ++i) {
        vertex(topRadius*cos(angle), 0, topRadius*sin(angle));
        vertex(bottomRadius*cos(angle), tall, bottomRadius*sin(angle));
        angle += angleIncrement;
    }
    endShape();
    
    // If it is not a cone, draw the circular top cap
    if (topRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);
        
        // Center point
        vertex(0, 0, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(topRadius * cos(angle), 0, topRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }
  
    // If it is not a cone, draw the circular bottom cap
    if (bottomRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);
    
        // Center point
        vertex(0, tall, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(bottomRadius * cos(angle), tall, bottomRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }
}