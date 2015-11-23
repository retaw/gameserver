drop table if exists t;
create table t
(
    id    integer NOT NULL primary key,
    num   integer default 0,
    str   varchar(32) default ""
);
