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

std::mutex m;

CerCon cont(5);
std::vector<double> lastState(5);

void drawCartAndRod(int x, float rodAngle, sf::RenderWindow& app) {
    sf::RectangleShape cart(sf::Vector2f(CART_W, CART_H));
    cart.setOrigin(CART_W/2, CART_H/2);
    cart.setPosition(x, RAIL_Y);
    app.draw(cart);

    sf::RectangleShape rod(sf::Vector2f(ROD_THICKNESS, ROD_LEN));
    rod.setOrigin(ROD_THICKNESS/2, ROD_LEN);
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

double controllerFunc(double cart_pos, double pen_angle, double cart_speed, double pen_speed, double pen_len )
{
    //Their controller code goes here
    //Returns a force on the cart
    double kp = 15;
    double error_deg;
    if (pen_angle < 180)
        error_deg = pen_angle;
    else
        error_deg = pen_angle - 360;
    double pkp = kp * error_deg;

    cerr << "Pendulum Error=" << error_deg << endl;
    double error = error_deg;
    cont.train( lastState, error );

    std::vector< double > curState = { cart_pos, pen_angle, cart_speed, pen_speed, pen_len };
    double ret = cont.predict( curState );
    cerr << "CerCon returned: " << ret << endl;

    // return 10.0 * ( (double)rand() / (double)RAND_MAX );

    lastState = curState;
    return ret + pkp;

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
        m.unlock();

        double returnedForce = controllerFunc(cart_pos_local, pen_angle_local, cart_speed_local, pen_speed_local, pen_len );

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
    sf::RenderWindow app(sf::VideoMode(640, 480), "SFML window");

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
        }

        m.lock();
        int cart_pos = (int)(engine.Get_cart_pos() * 100);
        int pen_angle = (int)engine.Get_pen_angle();
        m.unlock();

        app.clear();
        drawCartAndRod(cart_pos + 500, pen_angle, app);
        app.display();

        cerr << "****** CART_POS = " << engine.Get_cart_pos() << " PEN_LEN=" << engine.Get_pen_len() << endl;

        if( fabs(engine.Get_cart_pos()) > 2.0 )
        {
            engine.Set_pen_angle(1);
            engine.Set_cart_vel(0);
            engine.Set_cart_pos(0);
            engine.Set_pen_angular_vel(0);
            engine.Set_pen_angle( -15.0 + ((double)rand() / (double)RAND_MAX) * 80.0 );
//            double rndLen = ((double)rand() / (double)RAND_MAX) * 0.5;
//            engine.Set_pen_len( 0.6 + rndLen );
        }

        sf::sleep(sf::seconds(GRAPHICS_ENGINE_FREQUENCY_SEC));
    }
    physicsRunning = false;
    controllerRunning = false;

    physicsThread.join();
    controllerThread.join();

    return EXIT_SUCCESS;
}
