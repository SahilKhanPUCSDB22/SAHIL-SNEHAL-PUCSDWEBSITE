drop table sfiles;
drop table ifiles;
drop table sessions;

create table sessions
(
	session_id text primary key,
	userid varchar(1024),
	usertype text
); 

create table ifiles 
(
fname varchar(1024) primary key,
ftype varchar(100),
fsize int,
parts int,
owner varchar(1024) references interviewers(interviewers_id),
filedata text
);

create table sfiles 
(
fname varchar(1024) primary key,
ftype varchar(100),
fsize int,
parts int,
owner varchar(1024) references students(stud_id),
filedata text
);
