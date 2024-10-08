
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

#include <functional>
#include "cfg.h"
using namespace ftxui;

Component QuitTab(std::function<void()> quit) {
  return Button("Quit", quit, ButtonOption::Animated(Color::Cyan1));
}

Element MainMenuDecorator(Element element) {
  return vbox({
      text("Cpp Game") | bold | center,
      element,
  });
}

// A tab menu, with extra wide items.
MenuOption CustomMenuOption() {
  auto option = MenuOption::HorizontalAnimated();

  option.entries_option.transform = [](const EntryState& state) {
    Element e = text(state.label) | borderEmpty;
    if (state.focused) {
      //e |= inverted;
    }
    if (state.active) {
      e |= bold;
    }
    if (!state.focused && !state.active) {
      e |= dim;
    }
    return e;
  };

  return option;
};

const std::vector<std::string> g_levels_name = {
  "1",
  "2",
};
const std::vector<int> g_prize = {
    1,   //
    2,   //
};

//Component PlayTab(std::function<void(int)> play) {}

int LifeCost(const GameConfig& config){
    return config.difficulty;
}

namespace addInfo
{
static const char sep = '\x01';
static const char* const space = "  ";

static
void withAdditionalInfo(std::string& msg, const std::string& additional)
{
    if (additional.empty())
        return;
    const size_t pos = msg.rfind(sep);
    if (pos != std::string::npos)
      msg.resize(pos);
    msg += sep;
    msg += space;
    msg += additional;
}
  
} // namespace addInfo

Component RestoreTab(GameConfig& config,
        std::function<void(int)> play,
        ftxui::Closure quitThisPage, ftxui::Closure on_start_new_game) {
    struct Impl : public ComponentBase {
      Impl(GameConfig& config, std::function<void(int)> play,
        ftxui::Closure quitThisPage, ftxui::Closure on_start_new_game):
            config(config) {
          new_game = [this, on_start_new_game, quitThisPage, play] {
              this->config.backup->del();
              this->config.tryLoadBackup();
              quitThisPage();
              play(1);
              on_start_new_game();
          };
          oAddStr = "从最近存档开始";
          obtn = Button(&oAddStr, [this, quitThisPage, play] {
            if(!this->config.tryLoadBackup()){
                // no old data
                assert(false); // this shall not appear (see below)
            }
            play(2);
            quitThisPage();
          });
            nbtn = Button("开始新游戏", new_game);

          auto buttons = Container::Vertical({
                  obtn,
                  nbtn
          });
          auto renderer = Renderer(buttons, [this] {
                if (this->config.backup->hasData()) {
                    auto addInfo = this->config.backup->getAdditionalInfo();
                    addInfo::withAdditionalInfo(this->oAddStr, addInfo);
                    return vbox({
                      this->obtn->Render(),
                      this->nbtn->Render(),
                    });
                }
                return vbox({
                    this->nbtn->Render(),
                    text("尚无存档") | border
                });
          });
          Add(renderer);
      }

    private:
      // addidional str for obtn
      std::string oAddStr;
      Component obtn;
      Component nbtn;
      GameConfig& config;
      std::function<void()> new_game;
    };
    return Make<Impl>(config, std::move(play), std::move(quitThisPage), on_start_new_game);
}


Component MainMenu(GameConfig& config,
                   std::function<void(int)> play,
                   std::function<void()>  quitThisPage,
                   std::function<void()> quit,
                   std::function<void()> on_start_new_game) {
  static const std::vector<std::string> tab_entries_ = {
      "Play",
      "Quit",
  };

  class Impl : public ComponentBase {
   private:
    int tab_index_ = 0;
    GameConfig& config_;

   public:
    Impl(GameConfig& config,
         std::function<void(int)> play,
         ftxui::Closure quitThisPage,
         ftxui::Closure quit,
         ftxui::Closure on_start_new_game)
        : config_(config) {
      auto menu = Menu(&tab_entries_, &tab_index_, CustomMenuOption());
      auto tab_content = Container::Tab(
          {
              RestoreTab(config_, play, quitThisPage, on_start_new_game),
              QuitTab(quit),
          },
          &tab_index_);
      auto component = Container::Vertical({
          menu,
          tab_content,
      });
      component |= MainMenuDecorator;
      Add(component);
    }
  };

  return Make<Impl>(config, std::move(play), std::move(quitThisPage), std::move(quit), std::move(on_start_new_game));
}
