#!/usr/bin/perl
use strict;
use CGI qw(:standard);
use diagnostics;

mkdir $ENV{'UPLOAD_DIR'} unless -d $ENV{UPLOAD_DIR};
my $outfile = $ENV{'UPLOAD_DIR'}.$ENV{'FILE_NAME'};
my @a = (1..100);
my $filename = $outfile;
my $og_outfile = $outfile;
for(@a){
    if (-e $filename){
        $outfile = $og_outfile.$_;     #file exists
    }
    $filename = $og_outfile.$_;
}
open(my $fh, ">>", $outfile)
    or die "Can not open file: $!";
my $content = $ENV{'FILE_BODY'};
print $fh $content;
print <<ENDTAG;
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Webserv</title>
    <link rel="icon" href="/images/favicon-32x32.ico">
</head>
<body style="background-color:rgb(67, 67, 67);">
    <h1 style="text-align:center;font-size: 52px"><a href="/index" style="text-decoration: none;color: white">WEBSERV</a></h1><br><br>
        <p1 style=";color:rgb(255, 255, 255);font-size: 26px;"><center>Thanks for uploading a file!</p1><br>
        <p1 style=";color:rgb(255, 255, 255);font-size: 26px;">Your file is saved as $outfile</center></p1><br>
</body>
</html>
ENDTAG