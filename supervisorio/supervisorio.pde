import processing.net.*; 
import java.net.*;

Client myClient; 
String dataIn;
String[] dataSplit;
String[] pastData = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};

float gyrx, gyry, gyrz;
float accx, accy, accz;
float magx, magy, magz;
float kalx, kaly, kalz;

int i = 0;

void setup() {
    // define tamanho da janela e modo de rendering 3D
    size(1000, 600, P3D);

    // setup lights and antialiasing
    lights();
    smooth();

    // Connect to the local machine at port 9001.
    // This example will not run if you haven't
    // previously started a server on this port.
    myClient = new Client(this, "192.168.200.1", 9001);
}

void draw() {
    // muda tipo de perspectiva para ortho
    ortho();
    
    // black background
    background(0);
  
    if (myClient.available() > 0) {
        dataIn = myClient.readString();
        
        dataSplit = dataIn.split(";");
        for (i = 0; i < 12; i++){
            if (dataSplit[i].contains("nan")){
                dataSplit[i] = pastData[i];
            }
        }
        gyrx = radians(Float.parseFloat(dataSplit[0])+180);   gyry = radians(Float.parseFloat(dataSplit[1]));   gyrz = radians(-Float.parseFloat(dataSplit[2]));
        accx = radians(Float.parseFloat(dataSplit[3])+180);   accy = radians(Float.parseFloat(dataSplit[4]));   accz = radians(-Float.parseFloat(dataSplit[5]));
        magx = radians(Float.parseFloat(dataSplit[6])+180);   magy = radians(Float.parseFloat(dataSplit[7]));   magz = radians(-Float.parseFloat(dataSplit[8]));
        kalx = radians(Float.parseFloat(dataSplit[9])+180);   kaly = radians(Float.parseFloat(dataSplit[10]));  kalz = radians(-Float.parseFloat(dataSplit[11]));
        for (i = 0; i < 12; i++){
            pastData[i] = dataSplit[i];
        }
        
        myClient.write("abacaxi");
        delay(300);
    } else {
        for (i = 0; i < 12; i++){
            dataSplit[i] = pastData[i];
        }  
        gyrx = radians(Float.parseFloat(dataSplit[0])+180);   gyry = radians(Float.parseFloat(dataSplit[1]));   gyrz = radians(-Float.parseFloat(dataSplit[2]));
        accx = radians(Float.parseFloat(dataSplit[3])+180);   accy = radians(Float.parseFloat(dataSplit[4]));   accz = radians(-Float.parseFloat(dataSplit[5]));
        magx = radians(Float.parseFloat(dataSplit[6])+180);   magy = radians(Float.parseFloat(dataSplit[7]));   magz = radians(-Float.parseFloat(dataSplit[8]));
        kalx = radians(Float.parseFloat(dataSplit[9])+180);   kaly = radians(Float.parseFloat(dataSplit[10]));  kalz = radians(-Float.parseFloat(dataSplit[11]));
    }
  
    // mostra valores
    fill(255);
    text("X", 10, 50);
    text("Y", 10, 100);
    text("Z", 10, 150);
    fill(0, 255, 255);
    text(dataSplit[0], 50, 50);
    text(dataSplit[1], 50, 100);
    text(dataSplit[2], 50, 150);
    fill(255, 255, 0);
    text(dataSplit[3], 150, 50);
    text(dataSplit[4], 150, 100);
    text(dataSplit[5], 150, 150);
    fill(255, 0, 255);
    text(dataSplit[6], 250, 50);
    text(dataSplit[7], 250, 100);
    text(dataSplit[8], 250, 150);
    fill(255);
    text(dataSplit[9], 350, 50);
    text(dataSplit[10], 350, 100);
    text(dataSplit[11], 350, 150);
    
    // labels
    textAlign(CENTER);
    textSize(16);
    fill(255, 255, 255);
    text("FILTRADO", width/2, height/8);
    fill(0, 255, 255);
    text("GIROSCÓPIO", width/4, 7*height/8);
    fill(255, 255, 0);
    text("ACELERÔMETRO", width/2, 7*height/8);
    fill(255, 0, 255);
    text("MAGNETÔMETRO", 3*width/4, 7*height/8);
    
    // INÍCIO ACELEROMETRO (meio baixo) ##############################################################################################################
    pushMatrix();
    
    translate(width / 2, 2 * height / 3);
    criaCorpo(accx, accy, accz, 255, 255, 0);
    
    popMatrix();
    // FIM ACELEROMETRO
    
    // INICIO GIROSCOPIO (esquerdo baixo) ##############################################################################################################
    pushMatrix();
    
    translate(width / 4, 2 * height / 3);
    criaCorpo(gyrx, gyry, gyrz, 0, 255, 255);
    
    popMatrix();
    // FIM GIROSCOPIO
    
    // INICIO MAGNETOMETRO (direita baixo) ##############################################################################################################
    pushMatrix();
    
    translate(3 * width / 4, 2 * height / 3);
    criaCorpo(magx, magy, magz, 255, 0, 255);
    
    popMatrix();
    // FIM MAGNETOMETRO
    
    // INICIO FILTRADO (meio cima) ##############################################################################################################
    pushMatrix();
    
    translate(width / 2, height / 3);
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