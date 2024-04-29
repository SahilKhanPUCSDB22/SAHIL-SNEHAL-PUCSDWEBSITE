drop table studenttest;
drop table test;

create table test(
	testid int primary key,
	testname text,
	sdate date,
	stime time,
	edate date,
	etime time,
	testtype varchar(30),
	questions text,
	creatorid int references interviewers(interviewers_id)
);	

create table studenttest(
	testid int references test(testid),
	studentid int references students(stud_id), 
	answer text ,
	submitdate date,
	submittime time,
	marks int,
	primary key(testid,studentid)
);	
