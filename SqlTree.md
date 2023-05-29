# SqlTable

```sql

-- The financial table holds the name data for each financial in the tree.

CREATE TABLE financial
    (
        id INTEGER PRIMARY KEY AUTOINCREMENT,

        name TEXT NOT NULL,

        description TEXT DEFAULT NULL
        );

-- The code creates a table financial_path. The table has 4 columns: id, ancestor, descendant, and distance. The table is used to store the path between two financials in a graph. The distance is the number of edges between two financials. The ancestor and descendant columns are foreign keys to the financial table. The ancestor column stores the id of the financial that is the ancestor of the descendant financial. The descendant column stores the id of the financial that is the descendant of the ancestor financial. The distance column stores the number of edges between the two financials.

CREATE TABLE financial_path
    (
        id INTEGER PRIMARY KEY AUTOINCREMENT,

        ancestor INTEGER NOT NULL,

        descendant INTEGER NOT NULL,

        distance TINYINT NOT NULL CHECK (distance >= 0),

        FOREIGN KEY (ancestor) REFERENCES financial(id),

        FOREIGN KEY (descendant) REFERENCES financial(id),

        UNIQUE (ancestor, descendant)

    );

CREATE TABLE financial_transaction
    (
        id INTEGER PRIMARY KEY AUTOINCREMENT,

        source INTEGER NOT NULL,

        note TEXT DEFAULT NULL,

        description TEXT DEFAULT NULL,

        target INTEGER NOT NULL,

        debit MONEY DEFAULT NULL,

        credit MONEY DEFAULT NULL,

        FOREIGN KEY (source) REFERENCES financial(id),

        FOREIGN KEY (target) REFERENCES financial(id)

    );

-- This code creates an index on the name column of the financial table.

CREATE INDEX financial_name_index
    ON  financial (name);

-- Insert some example data

INSERT INTO financial (name) VALUES ('A');
INSERT INTO financial (name) VALUES ('B');
INSERT INTO financial (name) VALUES ('C');
INSERT INTO financial (name) VALUES ('D');
INSERT INTO financial (name) VALUES ('E');
INSERT INTO financial (name) VALUES ('F');
INSERT INTO financial (name) VALUES ('G');
INSERT INTO financial (name) VALUES ('H');
INSERT INTO financial (name) VALUES ('I');
INSERT INTO financial (name) VALUES ('J');
INSERT INTO financial (name) VALUES ('K');
INSERT INTO financial (name) VALUES ('L');
INSERT INTO financial (name) VALUES ('M');

INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 1, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (2, 2, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (3, 3, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (4, 4, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (5, 5, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (6, 6, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (7, 7, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (8, 8, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (9, 9, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (10, 10, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (11, 11, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (12, 12, 0);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (13, 13, 0);

INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 2, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 3, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 4, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 5, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 6, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 7, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 8, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 9, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 10, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 11, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 12, 1);
INSERT INTO financial_path (ancestor, descendant, distance) VALUES (1, 13, 1);
```
