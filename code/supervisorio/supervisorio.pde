float gyrx, gyry, gyrz;
float accx, accy, accz;
float magx, magy, magz;
float kalx, kaly, kalz;

float i = 0;

void setup() {
    // define tamanho da janela e modo de rendering 3D
    size(1000, 600, P3D);

    // setup lights and antialiasing
    lights();
    smooth();
}

void draw() {
    i += 1;
    
    // definicao dos valores de angulo
    gyrx = radians(i);   gyry = radians(0);   gyrz = radians(0);
    accx = radians(0);   accy = radians(i);   accz = radians(0);
    magx = radians(0);   magy = radians(0);   magz = radians(i);
    kalx = radians(i);   kaly = radians(i);   kalz = radians(i);
    
    // muda tipo de perspectiva para ortho
    ortho();
    
    // black background
    background(0);
    
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