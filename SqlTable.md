# SqlTable

```sql

-- The node table holds the name data for each node in the tree.

CREATE TABLE node
    (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        description TEXT NULL
        );

-- The code creates a table node_path. The table has 4 columns: id, ancestor, descendant, and distance. The table is used to store the path between two nodes in a graph. The distance is the number of edges between two nodes. The ancestor and descendant columns are foreign keys to the node table. The ancestor column stores the id of the node that is the ancestor of the descendant node. The descendant column stores the id of the node that is the descendant of the ancestor node. The distance column stores the number of edges between the two nodes.

CREATE TABLE node_path
    (
        id INTEGER PRIMARY KEY AUTOINCREMENT,

        ancestor INTEGER NOT NULL,

        descendant INTEGER NOT NULL,

        distance TINYINT NOT NULL CHECK (distance >= 0),

        FOREIGN KEY (ancestor) REFERENCES node(id),

        FOREIGN KEY (descendant) REFERENCES node(id),

        UNIQUE (ancestor, descendant)

    );

-- This code creates an index on the name column of the node table.

CREATE INDEX node_name_index
    ON  node (name);

-- Insert some example data

INSERT INTO node (name) VALUES ('A');
INSERT INTO node (name) VALUES ('B');
INSERT INTO node (name) VALUES ('C');
INSERT INTO node (name) VALUES ('D');
INSERT INTO node (name) VALUES ('E');
INSERT INTO node (name) VALUES ('F');
INSERT INTO node (name) VALUES ('G');
INSERT INTO node (name) VALUES ('H');
INSERT INTO node (name) VALUES ('I');
INSERT INTO node (name) VALUES ('J');
INSERT INTO node (name) VALUES ('K');
INSERT INTO node (name) VALUES ('L');
INSERT INTO node (name) VALUES ('M');

INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 1, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (2, 2, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (3, 3, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (4, 4, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (5, 5, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (6, 6, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (7, 7, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (8, 8, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (9, 9, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (10, 10, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (11, 11, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (12, 12, 0);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (13, 13, 0);

INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 2, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 3, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 4, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 5, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 6, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 7, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 8, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 9, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 10, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 11, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 12, 1);
INSERT INTO node_path (ancestor, descendant, distance) VALUES (1, 13, 1);

```
