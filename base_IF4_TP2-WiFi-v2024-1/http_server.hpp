/*
 * http_server.hpp
*
 *  Created on: 13/12/2022
 *  Last Modification: 13/12/2022
 *      Author: Florian Bianco (florian.bianco@univ-lyon1.fr)
 *              Romain Delpoux (romain.delpoux@insa-lyon.fr)
 *              Arnaud Lelevé  (arnaud.leleve@insa-lyon.fr)
 */

#ifndef HTTP_SERVER_HPP_
#define HTTP_SERVER_HPP_

#include <WiFi.h>

//- Types globalux ----------------------------
enum _ORDER {
  ORDER_ROBOT_FORWARD = 0x11,
  ORDER_ROBOT_BACKWARD = 0x22,
  ORDER_ROBOT_LEFT = 0x21,
  ORDER_ROBOT_RIGHT = 0x12,
  ORDER_ROBOT_STOP = 0x33,
};


// Prototypes -----------------------
void wifi_start(const char * ssid, const char * password);

int communicate_with_phone();



#endif /* HTTP_SERVER_HPP_ */
