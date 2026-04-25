// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
// #include <memory>  // for allocator, __shared_ptr_access
#include <cstdio>
#include <string>  // for char_traits, operator+, string, basic_string

#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/captured_mouse.hpp>  // for ftxui
#include <ftxui/component/component.hpp>       // for Input, Renderer, Vertical
#include <ftxui/component/component_base.hpp>  // for ComponentBase
#include <ftxui/component/component_options.hpp>  // for InputOption
#include <ftxui/dom/elements.hpp>  // for text, hbox, separator, Element, operator|, vbox, border
#include <ftxui/util/ref.hpp>  // for Ref

#include <nlohmann/json.hpp>

int main() {
  using namespace ftxui;

  FILE *p;
  std::string querry;

  p = popen("gh api graphql -f query='query { viewer { projectsV2(first:100) { nodes { id title } } } }'","r");

  if( p == NULL)
  {
      puts("Unable to open process");
      return 1;
  }

  char line[256];
  while(fgets(line, 256, p) != nullptr)
      querry += line;
  pclose(p);

  auto renderer = Renderer([&]
  {
      return vbox(
      {
          paragraph(querry)
      })
      | border;
  });

  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(renderer);
}
