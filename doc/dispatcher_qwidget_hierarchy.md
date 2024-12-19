# Dispatcher QWidget Hierarchy 

- DispatcherWidget : QScrollArea
- QGroupBox w/ QVBoxLayout
  - QComboBox (selects configuration)
  - QSplitter
    - QGroupBox w/ QVBoxLayout
      - QGroupBox w/ QVBoxLayout (repeatable for a category of processes)
        - ToolButton
        - ScrollArea w/ QGridLayout
          - *multiple \{ros/shell\}_process_items*
      - QGroupBox w/ QGridLayout (repeatable for a single process)
        - *one \{ros/shell\}_process_item*
      - QGroupBox w/ QGridLayout
        - *one start/stop_all button*
    - QGroupBox w/QGridLayout
      - *two ScriptItem per row*
    - QGroupBox w/QGridLayout
      - *one VariableDropdown per row*