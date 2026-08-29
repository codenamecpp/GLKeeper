#pragma once

//////////////////////////////////////////////////////////////////////////

/*
     #                                                                                        #
     ###                                                                                    ###
     ####                                                                                  ####
     ######                                                                              ######
    ########                                                                            ########
    ##########                                                                        ##########
    ###########                                                                      ###########
    ####  #######                                                                  #######  ####
    ####    ######                                                                ######    ####
    ####     #######                                                            #######     ####
    ####      #######                                                          #######      ####
    ####        #######                                                      #######        ####
     ####        #######                                                    #######         ###
     ####          #######                                                #######          ####
     ####           #######             ####################             #######           ####
     ####     ##     ########  ######################################  ########     ##     ####
      ###     ######   ######################################################   ######     ###
      ####    #########  ##################################################  #########    ####
      ####    #####################      ##################      #####################    ####
       ####    #################            #############           #################    ####
        ####   ################              ##########               ###############   ####
         ####    ############        #        ########        #        ############    ####
         ######     #########      #####      ########      #####      #########     #####
           #######   ##########################################################   #######
             ##########################################################################
               ######################################################################
                        ######    #########             ##########    ######
                        ######     #######               #######      ######
                       ########      ####                 ####       ########
                       #########      ##                            #########
                       ###########                                ###########
                      ##############              ###           ##############
                            ##############       ######   ###############
                                 ##################################
                                      ########################
                                            ############
                                                 ###                                                      
    
*/

//////////////////////////////////////////////////////////////////////////
// Powered by IMP Engine :)
//////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

//////////////////////////////////////////////////////////////////////////

#pragma warning ( disable : 4351 ) // new behavior: elements of array will be default initialized
#pragma warning ( disable : 4201 ) // nonstandard extension used: nameless struct/union

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>
#include <crtdbg.h>

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
    #define cxx_assert(expr) _ASSERTE(expr)
    #define cxx_assert_once(expr) \
        { \
            static bool is_asserted = false; \
            if (!(expr) && !is_asserted) \
            { \
                is_asserted = true; \
                _ASSERTE(expr); \
            } \
        }
#else
    #define cxx_assert(expr)
    #define cxx_assert_once(expr)
#endif

//////////////////////////////////////////////////////////////////////////

// stdlib
#define _USE_MATH_DEFINES
#include <iosfwd>
#include <math.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <queue>
#include <list>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <iterator>
#include <memory_resource>
#include <cstdarg>
#include <array>
#include <variant>

// opengl
#include "GLEW/GL/glew.h"
#include "GLFW/glfw3.h"

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>

// project
#include "std_ext.h"
#include "std_stream_helpers.h"
#include "static_vector.h"
#include "span.h"
#include "uniqueptr.h"
#include "noncopyable.h"
#include "intersection.h"
#include "auxmath.h"
#include "auxmath_angle.h"
#include "enum_serialize.h"
#include "memory_istream.h"
#include "container_adapter.h"
#include "CommonTypes.h"
#include "strings.h"
#include "FileSystem.h"
#include "InputsState.h"
#include "GameProfile.h"
#include "GraphicsDefs.h"
#include "VertexFormat.h"
#include "Console.h"
#include "RenderDevice.h"
#include "Debug.h"
#include "TimeManager.h"
#include "GameDefs.h"
#include "Camera.h"
#include "ScenarioDefs.h"
#include "GameMapDefs.h"
#include "Random.h"