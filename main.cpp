/* ===================================================================== */
/* This file is part of Daredevil                                        */
/* Daredevil is a side-channel analysis tool                             */
/* Copyright (C) 2016                                                    */
/* Original author:   Paul Bottinelli <paulbottinelli@hotmail.com>       */
/* Contributors:      Joppe Bos <joppe_bos@hotmail.com>                  */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* any later version.                                                    */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/* ===================================================================== */
#include <omp.h>
#include <stdlib.h>
#include "utils.h"
#include "focpa.h"
#include "socpa.h"


template <class TypeTrace, class TypeReturn, class TypeGuess>
int attack(Config & conf)
{
    int res = -1;
    printf("[ATTACK] Computing %i-order correlations for %i-order moments and window size %i...\n", conf.attack_order, conf.attack_moment, conf.window);
    fflush(stdout);
    if(conf.attack_order == 1)
      res = first_order<TypeTrace, TypeReturn, TypeGuess>(conf); //first_order can be found in focpa.cpp
    else if(conf.attack_order == 2)
      res = second_order<TypeTrace, TypeReturn, TypeGuess>(conf); //second_order can be found in socpa.cpp
    else {
      printf("That attack order isn't implemented yet. \n");
      return res;
    }
    return res;
}

int run(Config & conf)
{

  if (conf.type_guess == 'u'){
    if (conf.type_return == 'd'){
      if (conf.type_trace == 'f'){
        return attack<float, double, uint8_t>(conf); //attack is defined above
      }else if (conf.type_trace == 'i'){
        return attack<int8_t, double, uint8_t>(conf);
      }else if (conf.type_trace == 'd'){
        return attack<double, double, uint8_t>(conf);
      //}else if (conf.type_trace == 'u'){
        //attack<uint8_t, double, uint8_t>(conf);
      }else{
        fprintf(stderr, "[ERROR] Unsupported trace type [%c].\n", conf.type_trace);
      }
    }else if (conf.type_return == 'f') {
      if (conf.type_trace == 'f')
//        attack<float, float, uint8_t>(conf);
        fprintf(stderr, "[ERROR] Unsupported Yet.\n");
      else if (conf.type_trace == 'i')
        return attack<int8_t, float, uint8_t>(conf);
      else
        fprintf(stderr, "[ERROR] Unsupported combination of trace and return type [%c, %c].\n", conf.type_trace, conf.type_return);
    }else
      fprintf(stderr, "[ERROR] Unsupported return type [%c].\n", conf.type_return);
  }else {
    fprintf(stderr, "[ERROR] Unsupported guess type [%c].\n", conf.type_guess);
  }
  return 0;
}

int main(int argc, char * argv[])
{
  int res = 0;
  char * config_path = NULL;
  double start, end;

  // Valgrind says might want to allocate the struct with calloc and check for NULL ptr. This requires rewriting all the struct assignment. ". => ->"
  Config conf; //The Config struct is found in utils.h
  res = parse_args(argc, argv, &config_path); //parse_args is found in utils.cpp
  if (res != 0) {
    fprintf(stderr, "[ERROR] Parsing arguments.\n");
    return -1;
  }
  if (config_path == NULL){
    fprintf(stderr, "[ERROR] Invalid config file value.\n");
    return -1;
  }

  res = load_config(conf, config_path); //load_config is found in utils.cpp

  if (res != 0) {
    fprintf(stderr, "[ERROR] loading configuration file.\n");
    return -1;
  }

  print_config(conf); //print_config is found in utils.cpp

  for (size_t i = 0; i < conf.all_sboxes.size(); i++) {
    res = parse_sbox_file(conf.all_sboxes[i].c_str(), &conf.sbox); //parse_sbox_file is found in utils.cpp
    if (res != 0){
        fprintf(stderr, "[ERROR]: Loading lookup table %s.\n", conf.all_sboxes[i].c_str());
        continue;
    }

    printf("[INFO] Lookup table specified at %s\n\n", conf.all_sboxes[i].c_str());
    start = omp_get_wtime();
    res = run(conf); //run i defined above
    end = omp_get_wtime();
    printf("[INFO] Total attack of file %s done in %lf seconds.\n\n", conf.all_sboxes[i].c_str(), end - start);
    fflush(stdout);
    if (res != 0)
      return res;
    free(conf.sbox);
  }
  return 0;
}
