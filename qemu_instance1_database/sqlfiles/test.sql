drop table fanswer;
drop table fquestion;
/*drop table danswer;
drop table manswer;
drop table studenttest;
drop table dquestion;
drop table options;
drop table mquestion;
drop table test;
*/

create table test 
(
	testid varchar(1024) primary key, 
	testname text,
	startdate date,
	starttime time,
	enddate date,
	endtime time,
	testtype char(1),
	studtype char(1),
	randomize char(1),
	no_of_attempts text,
	review char(1),
	teststatus char(1),
	creatorid varchar(1024) references interviewers(interviewers_id)
);

create table mquestion 
(
	testid varchar(1024),
	qno varchar(1024),
	ques text,
	marks text,
	primary key (testid,qno),
	foreign key (testid) references test(testid)
);

create table options 
(
	testid varchar(1024),
	qno varchar(1024),
	ono varchar(1024),
	optext text,
	answer varchar(1024),
	primary key (testid,qno,ono),
	foreign key (testid,qno) references mquestion(testid,qno)
);

create table dquestion
(
	testid varchar(1024),
	qno varchar(1024),
	ques text,
	marks integer,
	charlimit integer,
	primary key(testid,qno),
	foreign key (testid) references test(testid)
);

create table fquestion
(
	testid varchar(1024),
	qno varchar(1024),
	fname text ,
	fsize text , 
	parts text ,
	ques text ,
	fcontent text ,
	primary key(testid,qno),
	foreign key(testid) references test(testid)
);


create table studenttest
(
	studid varchar(1024),
	testid varchar(1024),
	submitdate date,
	submittime time,
	status text,
	marks integer,
	attempts text,
	primary key(studid,testid),
	foreign key (studid) references students(stud_id),
	foreign key (testid) references test (testid)
);

create table manswer
(
	studid varchar(1024),
	testid varchar(1024),
	qno varchar(1024) ,
	answer varchar(1024),
	primary key (studid,testid,qno),
	foreign key (studid,testid) references studenttest(studid,testid),
	foreign key (testid,qno) references mquestion(testid,qno)	
);

create table danswer(
	studid varchar(1024),
	testid varchar(1024),
	qno varchar(1024) ,
	answer text,
	primary key(studid,testid,qno),
	foreign key(studid,testid) references studenttest(studid,testid),	
	foreign key (testid,qno) references dquestion(testid,qno)	
);

create table fanswer(	
	studid varchar(1024),
	testid varchar(1024),
	qno varchar(1024),
	fname text,
	fsize text, 
	parts text,
	answer text,
	primary key(studid,testid,qno),
	foreign key(studid,testid) references studenttest(studid,testid),	
	foreign key (testid,qno) references fquestion(testid,qno)	
);


