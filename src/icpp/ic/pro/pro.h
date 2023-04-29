// Handles licensing of icpp-pro

/*
********************************************************************************
* IT IS NOT ALLOWED TO MODIFY THIS FILE OR ANY FILE THAT CALLS THESE FUNCTIONS *
********************************************************************************
*/

#pragma once

class Pro {
public:
  Pro();
  ~Pro();

  void exit_if_not_pro();

private:
  bool is_pro();
};