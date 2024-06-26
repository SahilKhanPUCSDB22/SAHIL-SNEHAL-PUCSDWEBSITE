drop table students ;
drop table colleges;
drop table interviewers;
drop table companies;

create table companies (
	unique_ID varchar,
	company_id varchar(1024) unique, 
	company_name varchar(100),
	company_location varchar(100),
	company_domain text);

create table colleges(
	unique_ID varchar,
	college_id varchar(1024) unique,
       	college_name varchar(100),
	college_location varchar(100),
	college_domain text);

create table interviewers(
	unique_ID varchar,
	interviewers_id varchar(1024) unique,
       	interviewers_fname varchar(100),
	interviewers_lname varchar(100),
	interviewers_role varchar(100),
	interviewers_password varchar(10),
	company_id varchar(1024) references companies(company_id));

create table students(
	unique_ID varchar,
	stud_id varchar(1024) unique, 
	stud_fname varchar(100),
	stud_lname varchar(100),
	stud_add varchar(100),
	stud_stream varchar(100),
	stud_password varchar(10),
	stud_degree text,
	college_id varchar(1024) references colleges(college_id));

alter table companies add constraint pk1 PRIMARY KEY (company_id);
alter table colleges add constraint pk2 PRIMARY KEY (college_id);
alter table interviewers add constraint pk3 PRIMARY KEY (interviewers_id);
alter table students add constraint pk4 PRIMARY KEY (stud_id);

alter table companies drop column unique_ID;
alter table colleges drop column unique_ID;
alter table interviewers drop column unique_ID;
alter table students drop column unique_ID;
