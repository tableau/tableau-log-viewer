#ifndef COLUMN_H
#define COLUMN_H

enum COL : short {
    ID = 0,
    File,
    Time,
    Elapsed,
    PID,
    TID,
    Severity,
    Request,
    Session,
    Site,
    User,
    Key,
    Value
};

inline const char* GetColumnName(COL column) {
    switch (column) {
       case ID: return "ID";
       case File: return "File";
       case Time: return "Time";
       case Elapsed: return "Elapsed";
       case PID: return "PID";
       case TID: return "TID";
       case Severity: return "Severity";
       case Request: return "Request";
       case Session: return "Session";
       case Site: return "Site";
       case User: return "User";
       case Key: return "Key";
       case Value: return "Value";
    }
    return "unknown column";
}

#endif // COLUMN_H
