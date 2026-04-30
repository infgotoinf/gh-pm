#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ftxui/component/animation.hpp>
#include <map>
#include <nlohmann/detail/string_concat.hpp>
#include <stddef.h>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/screen/color.hpp>

#include <nlohmann/json.hpp>


using namespace nlohmann;
using namespace ftxui;

constexpr static int FPS = 60;


enum GETRequest : uint8_t
{
    WITHOUT_OWNER_AND_REPO_VARS
  , WITH_OWNER_AND_REPO_VARS
};

std::string makeGETRequest(const std::string& query, const GETRequest get_request_type)
{
    std::string request = "gh api graphql -f query='" + query + "'";

    if (get_request_type == GETRequest::WITH_OWNER_AND_REPO_VARS)
        request += "-F owner='{owner}' -F name='{repo}'";

    FILE *p;
    p = popen(request.c_str(),"r");

    if( p == NULL)
    {
        puts("Unable to open process");
        exit(1);
    }

    std::string response;
    char line[256];
    while(fgets(line, sizeof(line), p) != nullptr)
        response += line;
    pclose(p);

    return response;
}

struct Field {
    std::string name;
    std::string description;
    char color[6];
};

struct FieldGroup {
    std::string name;
    std::map<std::string, Field> field;
};

struct View {
    FieldGroup vertical_field_group;
    std::string name;
    int number;
};


struct SubIssueSummary {
    int completed;
    int total;
    int percent_completed;
};

struct Issue {
    SubIssueSummary sub_issue_summary;
    std::string repository_name;
    std::string title;
    int number;
    bool is_closed;
    bool is_locked;
};


struct Lable {
    std::string name;
    char color[6];
};

struct Item {
    std::vector<Issue> issues;
    std::vector<Lable> labels;
    std::vector<Field> fields;
    bool is_archived;
};

struct Project {
    std::vector<Item> items;
    std::vector<View> views;
    std::string title;
    int number;
    Project(std::string t_title = "", int t_number = 0);
    void initFromGETRequest(int t_number);
};


std::vector<Project> fetchProjects()
{
    std::vector<Project> projects;
    const static std::string query =
R"(query {
  viewer {
    projectsV2(first:100) {
      nodes {
        title
        number
      }
    }
  }
})";
    const std::string response = makeGETRequest(query, GETRequest::WITHOUT_OWNER_AND_REPO_VARS);
    const json parsed_response = json::parse(response);
    for(auto& j : parsed_response["data"]["viewer"]["projectsV2"]["nodes"]) {
        // projects.push_back(Project{ .title  = j["title"]
        //                           , .number = j["number"] });
    }
    return projects;
}

// Project fetchSingleProject()
// {
//     Project project;
//     const static std::string query =
// R"(query($name: String!, $owner: String!) {
//   repository(name: $name, owner: $owner) {
//     projectsV2(first:100) {
//       nodes {
//         number
//         title
//       }
//     }
//   }
// })";
//     std::string response = makeGETRequest(query, GETRequest::WITH_OWNER_AND_REPO_VARS);
//     return project;
// }
Project fetchProject(int project_number)
{
    Project project;
    const std::string query =
R"(query {
  viewer {

    # Project
    projectV2(number:)" + std::to_string(project_number) + R"() {
      number
      title

      # Views
      views(first: 10) {
        nodes {
          name
          number
          layout
          verticalGroupByFields(first: 10) {
            nodes {
              ... on ProjectV2SingleSelectField {
                name
                options {
                  name
                  description
                  color
                }
              }
            }
          }
        }
      }

      # Project items
      items(first: 100) {
        nodes {
          content {

            # Issue specific content
            ... on Issue {
              repository { name }
              title
              number
              closed
              locked
              subIssuesSummary {
                completed
                total
                percentCompleted
              }
            }
          }

          # General item content
          isArchived
          fieldValues(first: 100) {
            nodes {
              ... on ProjectV2ItemFieldLabelValue {
                labels(first: 10) {
                  nodes {
                    name
                    color
                  }
                }
              }
              ... on ProjectV2ItemFieldSingleSelectValue {
                name
              }
            }
          }
        }
      }
    }
  }
})";
    const std::string response = makeGETRequest(query, GETRequest::WITHOUT_OWNER_AND_REPO_VARS);
    const json parsed_response = json::parse(response);
    auto j = parsed_response["data"]["viewer"]["projectV2"];
    // project = Project{ .title  = j["title"]
    //                  , .number = j["number"] };
    return project;
}


int main() {
    std::vector<Project> projects;

    int selected_project_index = 0;
    std::vector<std::string> project_name_vector = {};
    Component project_select_container = Container::Vertical({Menu(&project_name_vector
                                                                 , &selected_project_index)});

    int selected_project_item_index = 0;
    Project selected_project;
    Component project_container = Renderer([&] {
      // show a placeholder when no project is loaded
      if (selected_project.title.empty()) {
        return vbox({
          paragraph("No project loaded") | dim,
        }) | frame | yframe;
      }
      return vbox({
        text("Project") | bold,
        separator(),
        text("Number: " + std::to_string(selected_project.number)),
        text("Title: " + selected_project.title),
      }) | frame | yframe;
    });

    // --------------------------------------------------------------------------------------------
    // Project Select
    // --------------------------------------------------------------------------------------------
    Component project_select_page = Renderer(project_select_container, [&]
    {
        return vbox({
            project_select_container->Render() | frame,
        }) | yframe;
    });

    // --------------------------------------------------------------------------------------------
    // Project
    // --------------------------------------------------------------------------------------------
    Component project_page = Renderer(project_container, [&]
    {
        return vbox({
            project_container->Render() | frame,
        }) | yframe;
    });

    // --------------------------------------------------------------------------------------------
    // Tabs
    // --------------------------------------------------------------------------------------------
    int tab_index = 0;
    std::vector<std::string> tab_entries = {
        "Project Select",
        "Project",
    };

    MenuOption option = MenuOption::HorizontalAnimated();
    option.underline.SetAnimation(std::chrono::milliseconds(150),
                                animation::easing::CircularInOut);
    option.entries_option.transform = [](EntryState state)
    {
        Element e = text(state.label);
        if (state.active) {
            e = e | color(Color::Blue);
            if (state.focused)
                e = e | bold;
        }
        else {
            if (!state.focused)
                e = e | dim;
        }
        return e;
    };
    option.underline.color_inactive = Color::Default;
    option.underline.color_active = Color::Blue;

    option.on_change = ([&]{
        switch (tab_index) {
        case 0:
            projects = fetchProjects();
            project_name_vector.clear();
            for (Project p : projects)
                project_name_vector.push_back(std::to_string(p.number) + p.title);
            if (selected_project_index >= project_name_vector.size())
                selected_project_index = project_name_vector.size() - 1;
            break;

        default:
            selected_project = fetchProject(projects[selected_project_index].number);
        }
    });

    Component tab_selection = Menu(&tab_entries, &tab_index, option);
    Component tab_content = Container::Tab(
    {
        project_select_page,
        project_page,
    },
    &tab_index);

    Component main_container = Container::Vertical({
        Container::Horizontal({
            tab_selection,
        }),
        tab_content,
    });

    // --------------------------------------------------------------------------------------------
    // Main window
    // --------------------------------------------------------------------------------------------
    Component main_renderer = Renderer(main_container, [&] {
        return vbox(
        {
            text("Github Project Manager") | bold | italic | align_right
                | color(Color::Yellow),

            hbox({
                tab_selection->Render() | flex,
            }),
            tab_content->Render() | flex,
        });
    });

    main_renderer |= CatchEvent([&](Event event) {
        if (event == Event::K && tab_index > 0) {
            --tab_index;
            option.on_change();
            return true;
        }
        if (event == Event::J && tab_index < tab_entries.size() - 1) {
            ++tab_index;
            option.on_change();
            return true;
        }
        return false;
    });

    // --------------------------------------------------------------------------------------------
    // Render
    // --------------------------------------------------------------------------------------------
    option.on_change(); // Initial call

    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    Loop loop(&screen, main_renderer);
    while (!loop.HasQuitted()) {
        screen.RequestAnimationFrame();
        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
    }

    return 0;
}
