^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package dispatcher
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
-----------

0.4.2 (2026-08-31 07:32)
------------------------
* Merge pull request `#3 <https://github.com/nasa-jpl/dispatcher/issues/3>`_ from nasa-jpl/fix-rolling

  Fix rolling
* add changelo update
* use target_link_libraries instead of deprecated ament_target_dependencies
* Contributors: Daniel Pastor

0.4.0 (2026-06-28 20:40)
------------------------
* chore: bump version to 0.4.0 [skip ci]
* Merge pull request `#1 <https://github.com/nasa-jpl/dispatcher/issues/1>`_ from nasa-jpl/github-ci
  add actions and workflows
* add actions and workflows
* Contributors: Daniel Pastor, github-actions[bot]

0.3.0 (2026-06-28 20:02)
------------------------
* Merge branch 'prepare-public-release' into 'master'
  update license to Apache. Readme corrections. Manually bump version
  See merge request robotics/ros2/dispatcher!4
* update license to Apache. Readme corrections. Manually bump version
* Merge branch 'feature/script_categories' into 'master'
  Add categories to scripts
  See merge request robotics/ros2/dispatcher!3
* doc image resize
* Updated docs to mention script categories
* Added attribution-free icons
* categories extended to scripts
* Contributors: Daniel Pastor, Daniel Pastor Moreno, jongreen

0.2.0 (2026-04-24)
------------------
* manual version bump
* Merge branch 'improve_readme' into 'master'
  Improve readme
  See merge request robotics/ros2/dispatcher!2
* fix tests
* add more examples
* add more examples with screenshots
* make start checked optional
* Merge branch 'casah-less' into 'master'
  Update to work without CASAH repos
  See merge request robotics/ros2/dispatcher!1
* remove debugging prints
* make namespace optional in config
* add github CI
* correct readme, add missing features
* add function doxygen comments
* add basic testing
  refactor logic for easier testing
* fix Q_OBJECT warnings
* fix smaller issues
  - missing regex header
  - missing h files in lib for QT to work
  - add GetTimerRate
* use ros params
* replace EVRs with RCLCPPs logging
* update CMakeLists.txt
* move headers to their own folder
* Contributors: Daniel Pastor, Daniel Pastor Moreno, Joseph Bowkett

0.1.5 (2026-02-08)
------------------
* Merge branch 'initial_config_danielm' into 'master'
  add initial configuration param
  See merge request casah/ros2/dispatcher!45
* Merge branch 'initial_config_danielm' into 'master'
  add initial configuration param
  See merge request casah/ros2/dispatcher!45
* add initial configuration param
* Contributors: Casah Gitlab Runner, Daniel Pastor, Daniel Pastor Moreno

0.1.4 (2025-10-09)
------------------
* Merge branch 'prestonr-dispatcher-lock-file-deletion-on-error' into 'master'
  Small change
  See merge request casah/ros2/dispatcher!44
* Merge branch 'prestonr-dispatcher-lock-file-deletion-on-error' into 'master'
  Small change
  See merge request casah/ros2/dispatcher!44
* Small change
* Contributors: Casah Admin, Casah Gitlab Runner, Preston Rogers

0.1.3 (2025-06-12 04:39)
------------------------
* Merge branch 'prestonr-jazzy-ci-yml' into 'master'
  Initial commit
  See merge request casah/ros2/dispatcher!43
* Contributors: Casah Gitlab Runner

0.1.2 (2025-06-12 03:13)
------------------------
* Merge branch 'prestonr-jazzy-ci-yml' into 'master'
  Initial commit
  See merge request casah/ros2/dispatcher!43
* Merge branch 'prestonr-jazzy-ci-yml' into 'master'
  Initial commit
  See merge request casah/ros2/dispatcher!43
* Initial commit
* Contributors: Casah Gitlab Runner, Preston Rogers

0.1.1 (2025-03-18)
------------------
* Merge branch 'kwehage-u24' into 'master'
  Use yaml-cpp library, updates for U24
  See merge request casah/ros2/dispatcher!41
* Merge branch 'kwehage-u24' into 'master'
  Use yaml-cpp library, updates for U24
  See merge request casah/ros2/dispatcher!41
* Use yaml-cpp library, updates for U24
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.1.0 (2024-12-19 22:24)
------------------------
* Merge branch 'dwai-amend-readme-for-categories' into 'master'
  Update file README.md
  See merge request casah/ros2/dispatcher!40
* Merge branch 'dwai-amend-readme-for-categories' into 'master'
  Update file README.md
  See merge request casah/ros2/dispatcher!40
* Update file README.md
* Contributors: Casah Gitlab Runner, Dennis Wai

0.0.43 (2024-12-19 22:09)
-------------------------
* Merge branch 'dwai-qol-update-many-process-items' into 'master'
  Enhance Dispatcher handling for many processes
  See merge request casah/ros2/dispatcher!39
* Merge branch 'dwai-qol-update-many-process-items' into 'master'
  Enhance Dispatcher handling for many processes
  See merge request casah/ros2/dispatcher!39
* Add an enum for different DispatcherItems in the code
* Replaced with std::string& where it made sense
* Add a QWidget hierarchy to docs
* Converted two function names to PascalCase
* Redid example.yaml to demonstrate elided text
* Update ID used in CSS styling
* hide_unconfigured_processes now treated as bool
  Also update wording on an EVR
* Small edit to config/*yaml to highlight features
* Just kidding, I cannot reuse Spacer items over and over in layouts since that leads to segfaults since pointers to them become null as we clean up. Replaced with factory method
* Add spacers around start_stop_all buttons and script and variable sections to center everything
* Add some documentation to example YAMLs
* Upon configuration change, go through each collapsible section and toggle its state to force a redraw of the process buttons. This is necessary if hide_unconfigured_processes is true
* Update to use wifi icon for offline and online mode. Updated styling again
* Enable ability to opt in on whether you will gray out or hide unconfigured items
* Add support to define custom icons for each named configuration
* Add capability to remember and then restore from saved WindowSettings
* Add ability to gray out un-enabled ProcessItems
* WIP, vertically aligned start and stop buttons
* Update example YAML with new category of processes
* Docstring update
* WIP some small refactoring
* Add a third spacer to center things out
* WIP, working processes box
* WIP, dispatcher GUI loads and runs, but all the buttons are really small?
* WIP refactored code in node and widget class, added support for type:category, but untested
* WIP, main widget is a scroll and I have expanding processes
* WIP stable dispatcher window with structure desired
* WIP, working Splitter dividing bars
* WIP commit, but working collapsible section of a ScrollArea
* Add code to auto add scroll bar to process items
* Contributors: Casah Gitlab Runner, Dennis Wai

0.0.42 (2024-10-31)
-------------------
* Merge branch 'dwai-update-casah-node-v1.0.0' into 'master'
  Update casah_node to v1.0.0
  See merge request casah/ros2/dispatcher!38
* Merge branch 'dwai-update-casah-node-v1.0.0' into 'master'
  Update casah_node to v1.0.0
  See merge request casah/ros2/dispatcher!38
* Update casah_node to v1.0.0
  Add GetTimerRate function to dispatcher_node in order to use protected
  base class function
* Contributors: Casah Gitlab Runner, Dennis Wai, Kristopher Wehage

0.0.41 (2024-10-24)
-------------------
* Merge branch 'dwai-new-abstract-class' into 'master'
  Add new abstract class to reuse code between script_items and dispatcher_items
  See merge request casah/ros2/dispatcher!36
* Merge branch 'dwai-new-abstract-class' into 'master'
  Add new abstract class to reuse code between script_items and dispatcher_items
  See merge request casah/ros2/dispatcher!36
* Add new abstract class to reuse code between script_items and dispatcher_items
* Contributors: Casah Gitlab Runner, Dennis Wai

0.0.40 (2024-07-29)
-------------------
* Update file LICENSE.md
* Update file LICENSE.md
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.39 (2024-07-12)
-------------------
* Merge branch 'prestonr-instance-file-check' into 'master'
  Prestonr instance file check
  See merge request casah/ros2/dispatcher!33
* Merge branch 'prestonr-instance-file-check' into 'master'
  Prestonr instance file check
  See merge request casah/ros2/dispatcher!33
* Prestonr instance file check
* Contributors: Casah Gitlab Runner, Preston Rogers

0.0.38 (2024-05-16)
-------------------
* Merge branch 'kubiak-arm64-build' into 'master'
  Remove x86-64 specific build flag to allow builds on arm64
  See merge request casah/ros2/dispatcher!31
* Merge branch 'kubiak-arm64-build' into 'master'
  Remove x86-64 specific build flag to allow builds on arm64
  See merge request casah/ros2/dispatcher!31
* Remove march flag
* Merge remote-tracking branch 'origin/master' into kubiak-arm64-build
* Remove x86-64 specific build flag to allow builds on arm64
* Contributors: Casah Gitlab Runner, Gerik Kubiak, Preston Rogers

0.0.37 (2024-05-07)
-------------------
* Merge branch 'kwehage-artifacts' into 'master'
  Add build artifacts
  See merge request casah/ros2/dispatcher!32
* Contributors: Casah Gitlab Runner

0.0.36 (2024-03-30 19:06)
-------------------------
* Merge branch 'kwehage-artifacts' into 'master'
  Add build artifacts
  See merge request casah/ros2/dispatcher!32
* Contributors: Casah Gitlab Runner

0.0.35 (2024-03-30 15:50)
-------------------------
* Merge branch 'kwehage-artifacts' into 'master'
  Add build artifacts
  See merge request casah/ros2/dispatcher!32
* Merge branch 'kwehage-artifacts' into 'master'
  Add build artifacts
  See merge request casah/ros2/dispatcher!32
* Add build artifacts
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.34 (2023-12-21)
-------------------
* Merge branch 'kwehage-variables' into 'master'
  Add support for variables in config files
  See merge request casah/ros2/dispatcher!30
* Merge branch 'kwehage-variables' into 'master'
  Add support for variables in config files
  See merge request casah/ros2/dispatcher!30
* Add variable definition
* Add example config demonstrating variables
* Add support for variables
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.33 (2023-08-14)
-------------------
* Merge branch 'kwehage-return-value' into 'master'
  Ignore return value of system call
  See merge request casah/ros2/dispatcher!29
* Merge branch 'kwehage-return-value' into 'master'
  Ignore return value of system call
  See merge request casah/ros2/dispatcher!29
* Ignore return value of system call
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.32 (2023-08-10)
-------------------
* Merge branch 'kwehage-ignore-return-value' into 'master'
  Ignore return value of system call
  See merge request casah/ros2/dispatcher!28
* Merge branch 'kwehage-ignore-return-value' into 'master'
  Ignore return value of system call
  See merge request casah/ros2/dispatcher!28
* Ignore return value of system call
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.31 (2023-06-08)
-------------------
* Merge branch 'kwehage-list-nodes' into 'master'
  List online nodes on status light tool tip
  See merge request casah/ros2/dispatcher!27
* Merge branch 'kwehage-list-nodes' into 'master'
  List online nodes on status light tool tip
  See merge request casah/ros2/dispatcher!27
* List online nodes on status light tool tip
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.30 (2023-03-29)
-------------------
* Merge branch 'kwehage-hotfix' into 'master'
  Add missing return after rejecting StartCb for offline nodes
  See merge request casah/ros2/dispatcher!26
* Merge branch 'kwehage-hotfix' into 'master'
  Add missing return after rejecting StartCb for offline nodes
  See merge request casah/ros2/dispatcher!26
* Add missing return after rejecting StartCb for offline nodes
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.29 (2023-03-18)
-------------------
* Merge branch 'kwehage-start-all-fix' into 'master'
  Don't start node if its disabled
  See merge request casah/ros2/dispatcher!25
* Merge branch 'kwehage-start-all-fix' into 'master'
  Don't start node if its disabled
  See merge request casah/ros2/dispatcher!25
* Don't start node if its disabled
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.28 (2023-03-15)
-------------------
* Merge branch 'kwehage-configuration-fix' into 'master'
  More robust configuration handling
  See merge request casah/ros2/dispatcher!24
* Merge branch 'kwehage-configuration-fix' into 'master'
  More robust configuration handling
  See merge request casah/ros2/dispatcher!24
* Fix segfault when missing configuration; support multiple conventions for specifying configurations
* Contributors: Casah Gitlab Runner, David Kim, Kristopher Wehage

0.0.27 (2023-03-08)
-------------------
* Merge branch 'kwehage-segfault-hotfix' into 'master'
  UpdateConfigurations at startup
  See merge request casah/ros2/dispatcher!23
* Merge branch 'kwehage-segfault-hotfix' into 'master'
  UpdateConfigurations at startup
  See merge request casah/ros2/dispatcher!23
* UpdateConfigurations at startup
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.26 (2023-03-01)
-------------------
* Merge branch 'kwehage-configure-environment-variables' into 'master'
  Better online/offline mode support
  See merge request casah/ros2/dispatcher!22
* Merge branch 'kwehage-configure-environment-variables' into 'master'
  Better online/offline mode support
  See merge request casah/ros2/dispatcher!22
* Better online/offline mode support
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.25 (2023-02-02 20:48)
-------------------------
* Merge branch 'kwehage-bump-casah-node' into 'master'
  Bump casah node version
  See merge request casah/ros2/dispatcher!21
* Merge branch 'kwehage-bump-casah-node' into 'master'
  Bump casah node version
  See merge request casah/ros2/dispatcher!21
* Bump casah node version
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.24 (2023-02-02 20:20)
-------------------------
* Merge branch 'kwehage-ssh-timeout' into 'master'
  Add configurable ssh timeout
  See merge request casah/ros2/dispatcher!20
* Merge branch 'kwehage-ssh-timeout' into 'master'
  Add configurable ssh timeout
  See merge request casah/ros2/dispatcher!20
* Add configurable ssh timeout
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.23 (2023-01-24)
-------------------
* Merge branch 'kwehage-remote-sessions' into 'master'
  Support remote sessions and environment variables
  See merge request casah/ros2/dispatcher!19
* Merge branch 'kwehage-remote-sessions' into 'master'
  Support remote sessions and environment variables
  See merge request casah/ros2/dispatcher!19
* Support remote sessions and environment variables
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.22 (2022-12-23)
-------------------
* Merge branch 'kwehage-humble-ci' into 'master'
  Add humble CI
  See merge request casah/ros2/dispatcher!18
* Merge branch 'kwehage-humble-ci' into 'master'
  Add humble CI
  See merge request casah/ros2/dispatcher!18
* Add humble CI
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.21 (2022-12-22)
-------------------
* Merge branch 'kwehage-configuration' into 'master'
  Add support for configurations
  Closes `#17 <https://github.com/nasa-jpl/dispatcher/issues/17>`_ and `#18 <https://github.com/nasa-jpl/dispatcher/issues/18>`_
  See merge request casah/ros2/dispatcher!17
* Merge branch 'kwehage-configuration' into 'master'
  Add support for configurations
  Closes `#17 <https://github.com/nasa-jpl/dispatcher/issues/17>`_ and `#18 <https://github.com/nasa-jpl/dispatcher/issues/18>`_
  See merge request casah/ros2/dispatcher!17
* Add support for configurations
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.20 (2022-05-24)
-------------------
* Merge branch 'kwehage-casah-node-update' into 'master'
  Update to latest casah node
  See merge request casah/ros2/dispatcher!16
* Merge branch 'kwehage-casah-node-update' into 'master'
  Update to latest casah node
  See merge request casah/ros2/dispatcher!16
* Update to latest casah node
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.19 (2022-02-28)
-------------------
* Merge branch 'kwehage-cfw-cmake' into 'master'
  Use packaged CfwCmake
  See merge request casah/ros2/dispatcher!15
* Merge branch 'kwehage-cfw-cmake' into 'master'
  Use packaged CfwCmake
  See merge request casah/ros2/dispatcher!15
* Use packaged CfwCmake
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.18 (2021-08-24 03:10)
-------------------------
* Merge branch 'kwehage-dependencies' into 'master'
  Bump dependencies
  See merge request casah/ros2/dispatcher!14
* Merge branch 'kwehage-dependencies' into 'master'
  Bump dependencies
  See merge request casah/ros2/dispatcher!14
* Bump dependencies
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.17 (2021-08-24 03:00)
-------------------------
* Merge branch 'kwehage-show-config' into 'master'
  Display config in titlebar
  Closes `#10 <https://github.com/nasa-jpl/dispatcher/issues/10>`_ and `#9 <https://github.com/nasa-jpl/dispatcher/issues/9>`_
  See merge request casah/ros2/dispatcher!13
* Merge branch 'kwehage-show-config' into 'master'
  Display config in titlebar
  Closes `#10 <https://github.com/nasa-jpl/dispatcher/issues/10>`_ and `#9 <https://github.com/nasa-jpl/dispatcher/issues/9>`_
  See merge request casah/ros2/dispatcher!13
* Display config in titlebar
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.16 (2021-08-18)
-------------------
* Merge branch 'abrinkma-customize-stop-signal' into 'master'
  add optional YAML field `stop_tmux_keys`
  See merge request casah/ros2/dispatcher!12
* Merge branch 'abrinkma-customize-stop-signal' into 'master'
  add optional YAML field `stop_tmux_keys`
  See merge request casah/ros2/dispatcher!12
* add optional YAML field to override the default tmux send keys stop command "C-C"
* Contributors: Alex Brinkman, Casah Gitlab Runner, Kristopher Wehage

0.0.15 (2021-05-27 21:45)
-------------------------
* Merge branch 'kwehage-evr-segfault' into 'master'
  Kwehage evr segfault
  Closes `#8 <https://github.com/nasa-jpl/dispatcher/issues/8>`_
  See merge request casah/ros2/dispatcher!11
* Merge branch 'kwehage-evr-segfault' into 'master'
  Kwehage evr segfault
  Closes `#8 <https://github.com/nasa-jpl/dispatcher/issues/8>`_
  See merge request casah/ros2/dispatcher!11
* Kwehage evr segfault
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.14 (2021-05-27 17:16)
-------------------------
* Merge branch 'kwehage-dependencies' into 'master'
  Update casah_node
  See merge request casah/ros2/dispatcher!10
* Merge branch 'kwehage-dependencies' into 'master'
  Update casah_node
  See merge request casah/ros2/dispatcher!10
* Update casah_node
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.13 (2021-05-17)
-------------------
* Merge branch 'kwehage-evr' into 'master'
  Add evr publishing
  See merge request casah/ros2/dispatcher!9
* Merge branch 'kwehage-evr' into 'master'
  Add evr publishing
  See merge request casah/ros2/dispatcher!9
* Add evr publishing
* Contributors: Casah Gitlab Runner, Joseph Bowkett, Kristopher Wehage

0.0.12 (2021-04-25)
-------------------
* Merge branch 'kwehage-dependencies' into 'master'
  Dependencies
  See merge request casah/ros2/dispatcher!8
* Merge branch 'kwehage-dependencies' into 'master'
  Dependencies
  See merge request casah/ros2/dispatcher!8
* Dependencies
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.11 (2021-04-17 19:50)
-------------------------
* Merge branch 'kwehage-dependencies' into 'master'
  Bump dependencies
  See merge request casah/ros2/dispatcher!7
* Merge branch 'kwehage-dependencies' into 'master'
  Bump dependencies
  See merge request casah/ros2/dispatcher!7
* Bump dependencies
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.10 (2021-04-17 16:37)
-------------------------
* Merge branch 'kwehage-dependencies' into 'master'
  Bump dependencies
  See merge request casah/ros2/dispatcher!6
* Merge branch 'kwehage-dependencies' into 'master'
  Bump dependencies
  See merge request casah/ros2/dispatcher!6
* Bump dependencies
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.9 (2021-04-15)
------------------
* Merge branch 'apb-tmux-features' into 'master'
  Improve tmux features
  See merge request casah/ros2/dispatcher!5
* Merge branch 'apb-tmux-features' into 'master'
  Improve tmux features
  See merge request casah/ros2/dispatcher!5
* preserve name with spaces, only replace spaces in tmux name
* convert spaces to underscores in DispatcherItem name parameter
* MR comment cleanup; added proper getters DispatcherItem
* make format
* add underscore to tmux session names; cleaned up some syntax prefs; added clear C-U to terminal before running the StartCb()
* a more elegant way of handling non-uniqueness of tmux session-names
* WIP temp fix for -t target-session issue
* simple existence check significant improves tmux session handling
* WIP. working prototype for proposal to issue `#7 <https://github.com/nasa-jpl/dispatcher/issues/7>`_ insufficient session checking not yet implemented
* created several Tmux* methods to better observe DRY in callbacks; made some message DEBUG level
* Contributors: Alex Brinkman, Casah Gitlab Runner, Kristopher Wehage

0.0.8 (2021-04-07)
------------------
* Merge branch 'kwehage-certificate' into 'master'
  Install certificate
  See merge request casah/ros2/dispatcher!4
* Merge branch 'kwehage-certificate' into 'master'
  Install certificate
  See merge request casah/ros2/dispatcher!4
* Install certificate
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.7 (2021-04-06)
------------------
* Merge branch 'kwehage-user-scripts' into 'master'
  Adds user scripts
  See merge request casah/ros2/dispatcher!3
* Merge branch 'kwehage-user-scripts' into 'master'
  Adds user scripts
  See merge request casah/ros2/dispatcher!3
* Add script item
* Add script item
* Add application icon
* Add support for user scripts
* Add suport for user scripts
* Add more info prints
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.6 (2021-03-31)
------------------
* Merge branch 'kwehage-bump-dependencies' into 'master'
  Update deploy script
  See merge request casah/ros2/dispatcher!2
* Merge branch 'kwehage-bump-dependencies' into 'master'
  Update deploy script
  See merge request casah/ros2/dispatcher!2
* Update deploy script
* Merge branch 'kwehage-bump-dependencies' into 'master'
  Kwehage bump dependencies
  See merge request casah/ros2/dispatcher!1
* Clang format, add window title
* Add plot icon
* Bug fix in CMakeLists.txt
* Debugging
* Debugging
* Debugging
* Debuggin
* Debugging
* Bring in yaml_cpp_vendor in gitlab runner
* Try optionally bringing in yaml_cpp_vendor if not found
* Try yaml fallback
* Bump dependencies
* Use docker image
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.5 (2021-03-24 02:54)
------------------------
* Merge branch 'master' of fornat1.jpl.nasa.gov:casah/ros2/dispatcher
* Merge branch 'master' of fornat1.jpl.nasa.gov:casah/ros2/dispatcher
* Update README
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.4 (2021-03-24 02:43)
------------------------
* Add warning if gnome-terminal not found on system
* Add warning if gnome-terminal not found on system
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.3 (2021-03-24 02:29)
------------------------
* Merge branch 'master' of fornat1.jpl.nasa.gov:casah/ros2/dispatcher
* Merge branch 'master' of fornat1.jpl.nasa.gov:casah/ros2/dispatcher
* Swap order of name and namespace
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.2 (2021-03-24 01:58)
------------------------
* Merge branch 'master' of fornat1.jpl.nasa.gov:casah/ros2/dispatcher
* Merge branch 'master' of fornat1.jpl.nasa.gov:casah/ros2/dispatcher
* Add terminal.png icon
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.1 (2021-03-24 00:32)
------------------------
* Add back requests library
* Add back requests library
* Add qt5
* Use foxy base docker container
* Add dependencies
* Add qt5 to build environment
* Contributors: Casah Gitlab Runner, Kristopher Wehage

0.0.0 (2021-03-23)
------------------
* Shut down nodes gracefully when closing window
* Add ability to start gnome-terminal
* Add dispatcher graphic
* Add gitlab runner
* Whitespace
* Throw exception if tmux not found
* Use version numbers for dependencies
* Removed duplicate shutdown logic
* Start and stop nodes in detached tmux sessions
* Get online/offline status detection working
* Add subscribers
* Link icons library
* Initialize icons resource
* Handle sigint, get ros2 graph interface
* Add check box, graphic, and start/stop buttons
* Remove clang-format
* Initial commit
* Contributors: Kristopher Wehage
