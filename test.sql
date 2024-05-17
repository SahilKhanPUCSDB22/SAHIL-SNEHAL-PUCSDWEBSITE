drop table danswer;
drop table manswer;
drop table studenttest;
drop table dquestion;
drop table options;
drop table mquestion;
drop table test;

create table test 
(
	testid integer primary key, 
	testname text,
	startdate date,
	starttime time,
	enddate date,
	endtime time,
	testtype char(1),
	studtype char(1),
	randomize char(1),
	continue char(1),
	creatorid integer references interviewers(interviewers_id)
);

create table mquestion 
(
	testid integer,
	qno integer,
	ques text,
	marks integer,
	primary key (testid,qno),
	foreign key (testid) references test(testid)
);

create table options 
(
	testid integer,
	qno integer,
	ono integer,
	answer integer,
	primary key (testid,qno,ono),
	foreign key (testid,qno) references mquestion(testid,qno)
);

create table dquestion
(
	testid integer,
	qno integer,
	ques text,
	marks integer,
	primary key(testid,qno),
	foreign key (testid) references test(testid)
);

create table studenttest
(
	studid integer ,
	testid integer ,
	submitdate date,
	submittime time,
	status text,
	marks integer,
	primary key(studid,testid),
	foreign key (studid) references students(stud_id),
	foreign key (testid) references test (testid)
);

create table manswer
(
	studid integer,
	testid integer,
	qno integer ,
	answer integer,
	primary key (studid,testid,qno),
	foreign key (studid,testid) references studenttest(studid,testid),
	foreign key (testid,qno) references mquestion(testid,qno)	
);

create table danswer(
	studid integer ,
	testid integer ,
	qno integer ,
	answer text,
	primary key(studid,testid,qno),
	foreign key(studid,testid) references studenttest(studid,testid),	
	foreign key (testid,qno) references dquestion(testid,qno)	
);

insert into test values(1);
insert into test values(2);
insert into mquestion values(1,1);
insert into mquestion values(2,1);

