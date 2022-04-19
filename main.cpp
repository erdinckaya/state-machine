#include "state_machine.h"
#include <iostream>

struct AudioDecodeEvent {};
struct VideoDecodeEvent {};
struct AudioResumed {};
struct AudioPaused {};

#define GEN_STATE(name)                                                        \
  struct name final {                                                          \
                                                                               \
    struct InternalStartCond {                                                 \
      template <typename StateMachine>                                         \
      static bool check(const StateMachine &s, const VideoDecodeEvent &) {     \
        std::cout << __LINE__ << " InternalStartCond VideoDecodeEvent\n";      \
        return false;                                                          \
      }                                                                        \
                                                                               \
      template <typename StateMachine>                                         \
      static bool check(const StateMachine &s, const AudioDecodeEvent &) {     \
        std::cout << __LINE__ << " InternalStartCond AudioDecodeEvent\n";      \
        return true;                                                           \
      }                                                                        \
                                                                               \
      template <typename StateMachine, typename EventT>                        \
      static bool check(const StateMachine &s, const EventT &) {               \
        std::cout << __LINE__ << " InternalStartCond EventT\n";                \
        return false;                                                          \
      }                                                                        \
    };                                                                         \
                                                                               \
    struct InternalStartAction {                                               \
      template <typename StateMachine>                                         \
      static void execute(StateMachine &s, const VideoDecodeEvent &) {         \
        std::cout << __LINE__ << " InternalStartAction VideoDecodeEvent\n";    \
      }                                                                        \
      template <typename StateMachine>                                         \
      static void execute(StateMachine &s, const AudioDecodeEvent &) {         \
        std::cout << __LINE__ << " InternalStartAction AudioDecodeEvent\n";    \
      }                                                                        \
      template <typename StateMachine, typename EventT>                        \
      static void execute(StateMachine &s, const EventT &) {                   \
        std::cout << __LINE__ << " InternalStartAction EventT\n";              \
      }                                                                        \
    };                                                                         \
                                                                               \
    using InternalTable = sm::InternalStateMachineTable<                       \
        sm::InternalTableRow<VideoDecodeEvent, InternalStartAction,            \
                             InternalStartCond>,                               \
        sm::InternalTableRow<AudioDecodeEvent, InternalStartAction,            \
                             InternalStartCond>>;                              \
                                                                               \
    static void enter() {                                                      \
      std::cout << __LINE__ << " Entering " << #name << "\n";                  \
    }                                                                          \
    static void exit() {                                                       \
      std::cout << __LINE__ << " Exiting " << #name << "\n";                   \
    }                                                                          \
  }

GEN_STATE(Start);
GEN_STATE(Start1);
GEN_STATE(Start2);
GEN_STATE(Start3);
GEN_STATE(Start4);
GEN_STATE(Start5);
GEN_STATE(Start6);
GEN_STATE(Start7);
GEN_STATE(Start8);
GEN_STATE(Start9);

struct End final {
  using InternalTable = sm::InternalStateMachineTable<>;
  static void enter() { std::cout << __LINE__ << " Entering End\n"; }
  static void exit() { std::cout << __LINE__ << " Exiting End\n"; }
};

struct EndFSM {
  template <typename StateMachine>
  static void execute(StateMachine &machine, const VideoDecodeEvent &) {
    std::cout << __LINE__ << " Action EndFSM VideoDecodeEvent\n";
  }
  template <typename StateMachine>
  static void execute(StateMachine &machine, const AudioDecodeEvent &) {
    std::cout << __LINE__ << " Action EndFSM AudioDecodeEvent\n";
  }
  template <typename StateMachine>
  static void execute(StateMachine &machine, const AudioResumed &) {
    std::cout << __LINE__ << " Action EndFSM AudioResumed\n";
  }
  template <typename StateMachine>
  static void execute(StateMachine &machine, const AudioPaused &) {
    std::cout << __LINE__ << " Action EndFSM AudioPaused\n";
  }

  // Default Action
  template <typename StateMachine, typename Action>
  static void execute(StateMachine &machine, const Action &) {
    std::cout << __LINE__ << " Action EndFSM Action\n";
  }
};

struct StartFSM {
  template <typename StateMachine>
  static void execute(StateMachine &machine, const VideoDecodeEvent &) {
    std::cout << __LINE__ << " Action StartFSM VideoDecodeEvent\n";
  }
  template <typename StateMachine>
  static void execute(StateMachine &machine, const AudioDecodeEvent &) {
    std::cout << __LINE__ << " Action StartFSM AudioDecodeEvent\n";
  }
  template <typename StateMachine>
  static void execute(StateMachine &machine, const AudioResumed &) {
    std::cout << __LINE__ << " Action StartFSM AudioResumed\n";
  }
  template <typename StateMachine>
  static void execute(StateMachine &machine, const AudioPaused &) {
    std::cout << __LINE__ << " Action StartFSM AudioPaused\n";
  }

  // Default Action
  template <typename StateMachine, typename Action>
  static void execute(StateMachine &machine, const Action &) {
    std::cout << __LINE__ << " Action StartFSM Action\n";
  }
};

struct IsStart {
  template <typename StateMachine>
  static bool check(const StateMachine &, const VideoDecodeEvent &) {
    std::cout << __LINE__ << " IsStart VideoDecodeEvent\n";
    return true;
  }
  template <typename StateMachine>
  static bool check(const StateMachine &, const AudioDecodeEvent &) {
    std::cout << __LINE__ << " IsStart AudioDecodeEvent\n";
    return true;
  }

  // Add Default check.
  template <typename StateMachine, typename EventT>
  static bool check(const StateMachine &, const EventT &) {
    std::cout << __LINE__ << " IsStart EventT\n";
    return false;
  }
};

struct ReturnsTrue {
  template <typename StateMachine>
  static bool check(const StateMachine &, const VideoDecodeEvent &) {
    std::cout << __LINE__ << " ReturnsTrue VideoDecodeEvent\n";
    return true;
  }
  template <typename StateMachine>
  static bool check(const StateMachine &, const AudioDecodeEvent &) {
    std::cout << __LINE__ << " ReturnsTrue AudioDecodeEvent\n";
    return true;
  }

  template <typename StateMachine, typename EventT>
  static bool check(const StateMachine &, const EventT &) {
    std::cout << __LINE__ << " ReturnsTrue EventT\n";
    return false;
  }
};

int main() {
  // clang-format off

    // States have to be defined in advance!
    using States = sm::StatePack<Start, Start1, Start2, Start3, Start4, Start5, Start6, Start7, Start8, Start9, End>;
    using Table = sm::StateMachineTable<
            //===========CurrentEvent======CurrentState===NextState=====Action======Condition=====================W
            sm::TableRow<VideoDecodeEvent, Start,         Start1,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start,         Start1,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start1,        Start2,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start1,        Start2,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start2,        Start3,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start2,        Start3,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start3,        Start4,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start3,        Start4,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start4,        Start5,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start4,        Start5,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start5,        Start6,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start5,        Start6,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start6,        Start7,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start6,        Start7,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start7,        Start8,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start7,        Start8,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, Start8,        Start9,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, Start8,        Start9,       EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<AudioDecodeEvent, Start9,        End,          EndFSM,     sm::AND<IsStart, ReturnsTrue>>,
            sm::TableRow<VideoDecodeEvent, Start9,        End,          EndFSM,     sm::AND<IsStart, ReturnsTrue>>,

            sm::TableRow<VideoDecodeEvent, End,           Start,        StartFSM,   sm::OR<IsStart, ReturnsTrue>>,
            sm::TableRow<AudioDecodeEvent, End,           Start,        StartFSM,   sm::OR<IsStart, ReturnsTrue>>
            >;
  // clang-format on

  sm::StateMachine<States> machine{};
  machine.updateState(Start{});

  for (int i = 0; i < 100; ++i) {

    switch (auto index = i % 4; index) {
    case 0:
      sm::StateMachineEngine<Table>::onEvent(machine, VideoDecodeEvent{});
      break;
    case 1:
      sm::StateMachineEngine<Table>::onEvent(machine, AudioDecodeEvent{});
      break;
    case 2:
      sm::StateMachineEngine<Table>::onEvent(machine, AudioResumed{});
      break;
    case 3:
      sm::StateMachineEngine<Table>::onEvent(machine, AudioPaused{});
      break;
    default:
      break;
    }
    std::cout << "Iteration " << i << " =================\n";
  }

  return 0;
}
