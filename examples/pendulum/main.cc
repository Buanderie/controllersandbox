#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <thread>
#include <mutex>

#include "InvPendulumEngine.h"

#include <cercon.h>

using std::cout;
using std::endl;

const int ROD_LEN = 150;
const int ROD_THICKNESS = 5;
const int RAIL_Y = 400;
const int PIVOT_RADIAS = 10;
const int CART_W = 60;
const int CART_H = 40;

const double CONTROLLER_FREQUENCY_SEC = 0.001;
const double PHYSICS_ENGINE_FREQUENCY_SEC = 0.001;
const double GRAPHICS_ENGINE_FREQUENCY_SEC = 1.0 / 60; //~30 fps

bool physicsRunning = true;
bool controllerRunning = true;

double curRodLen = 1.0;

std::mutex m;

CerCon cont(3);
std::vector<double> lastState(3);

void drawCartAndRod(int x, float rodAngle, sf::RenderWindow& app) {
    sf::RectangleShape cart(sf::Vector2f(CART_W, CART_H));
    cart.setOrigin(CART_W/2, CART_H/2);
    cart.setPosition(x, RAIL_Y);
    app.draw(cart);

    sf::RectangleShape rod(sf::Vector2f(ROD_THICKNESS, ROD_LEN * curRodLen));
    rod.setOrigin(ROD_THICKNESS/2, ROD_LEN * curRodLen);
    rod.setPosition(x, RAIL_Y);
    rod.rotate(rodAngle);
    rod.setFillColor(sf::Color(100, 220, 50));
    app.draw(rod);

    sf::Vector2f origin = rod.getOrigin();
    sf::CircleShape pivot(PIVOT_RADIAS);
    pivot.setPosition(rod.getPosition() - sf::Vector2f(PIVOT_RADIAS,PIVOT_RADIAS));
    pivot.setFillColor(sf::Color(100, 220, 50));
    app.draw(pivot);
}

void physicsPeriodic(InvPendulumEngine* eng)
{
    double accumulator = 0;
    sf::Clock clock;
    sf::Time prev_time = clock.getElapsedTime();
    double timestep = PHYSICS_ENGINE_FREQUENCY_SEC;

    while(physicsRunning)
    {
        sf::Time curr_time = clock.getElapsedTime();
        sf::Time elapsed = curr_time - prev_time;
        double dt = (double)elapsed.asSeconds();
        prev_time = curr_time;
        accumulator += dt;

        while (accumulator >= timestep)
        {
            // step physics engine
            eng->step();
            accumulator -= timestep;
        }
        sf::sleep(sf::seconds(PHYSICS_ENGINE_FREQUENCY_SEC));
    }
}

double controllerFunc(double cart_pos, double pen_angle, double cart_speed, double pen_speed, double pen_len, double pen_mass )
{
    //Their controller code goes here
    //Returns a force on the cart
    double kp = 15;
    double error_deg;
    double error_speed = pen_speed;
    double cart_error = fabs(cart_pos);
    if (pen_angle < 180)
        error_deg = pen_angle;
    else
        error_deg = pen_angle - 360;
    double pkp = kp * error_deg;

//    pkp = 0;

    std::vector< double > curState = { pen_angle, cart_speed, pen_speed };
    cerr << "Pendulum Error=" << error_deg << endl;
    double error = error_deg;
    cont.train( lastState, error );

    double ret = cont.predict( curState );
    cerr << "CerCon returned: " << ret << endl;

    // return 10.0 * ( (double)rand() / (double)RAND_MAX );

    lastState = curState;
    double cret = ret + pkp;

    return cret;
}

void controllerPeriodic(InvPendulumEngine* eng)
{
    cout << "contollerPeriodic" << endl;
    while(controllerRunning)
    {
        m.lock();
        double cart_pos_local = eng->Get_cart_pos();
        double pen_angle_local = eng->Get_pen_angle();
        double cart_speed_local = eng->Get_cart_vel();
        double pen_speed_local = eng->Get_pen_angular_vel();
        double pen_len = eng->Get_pen_len();
        double pen_mass = eng->Get_pen_mass();
        m.unlock();

        double returnedForce = controllerFunc(cart_pos_local, pen_angle_local, cart_speed_local, pen_speed_local, pen_len, pen_mass );

        m.lock();
        eng->nextForce = returnedForce;
        m.unlock();

        sf::sleep(sf::seconds(CONTROLLER_FREQUENCY_SEC));
    }
}

int main()
{

    srand(time(NULL));

    // Create the main window
    sf::RenderWindow app(sf::VideoMode(1280, 720), "SFML window");

    InvPendulumEngine engine(&m);
    engine.Set_pen_angle(1);
    engine.Set_time_step(PHYSICS_ENGINE_FREQUENCY_SEC);

    std::thread physicsThread(physicsPeriodic, &engine);
    std::thread controllerThread(controllerPeriodic, &engine);

    // Start the game loop
    while (app.isOpen())
    {
        // Process events
        sf::Event event;
        while (app.pollEvent(event))
        {
            // Close window : exit
            if (event.type == sf::Event::Closed)
                app.close();

            if (event.type == sf::Event::KeyPressed)
                engine.nextForce = 30000;
        }

        m.lock();
        int cart_pos = (int)(engine.Get_cart_pos() * 100);
        int pen_angle = (int)engine.Get_pen_angle();
        m.unlock();

        app.clear();
        drawCartAndRod(cart_pos + 500, pen_angle, app);
        app.display();

        cerr << "****** CART_POS = " << engine.Get_cart_pos() << " PEN_LEN=" << engine.Get_pen_len() << endl;

        if( fabs(engine.Get_cart_pos()) > 6.0 )
        {
            engine.Set_pen_angle(1);
            engine.Set_cart_vel(0);
            engine.Set_cart_pos(0);
            engine.Set_pen_angular_vel(0);
            engine.Set_pen_angle( -15.0 + ((double)rand() / (double)RAND_MAX) * 30.0 );
                        engine.Set_pen_angle(180);
//            double rndLen = ((double)rand() / (double)RAND_MAX) * 2.5;
//            curRodLen = 0.2 + rndLen;
//            engine.Set_pen_len( curRodLen );
//            engine.Set_pen_mass( gngrand<double>(0.2, 100.0) );
        }

        sf::sleep(sf::seconds(GRAPHICS_ENGINE_FREQUENCY_SEC));
    }
    physicsRunning = false;
    controllerRunning = false;

    physicsThread.join();
    controllerThread.join();

    return EXIT_SUCCESS;
}
