#!/usr/bin/perl -wT
 
use strict;
use warnings;
use CGI;
use CGI::Carp qw(fatalsToBrowser);
use File::Basename;
 
$CGI::POST_MAX = 1024 * 5000; #adjust as needed (1024 * 5000 = 5MB)
$CGI::DISABLE_UPLOADS = 0; #1 disables uploads, 0 enables uploads
 
my $safe_filename_characters = "a-zA-Z0-9_.-";

my $upload_dir = "/var/www/html/nestor/uploads";

my $query = new CGI; 
my $filename = $query->param("modulo");

if ( !$filename ) 
{ 
   print $query->header ( ); 
   print "There was a problem uploading your photo (try a smaller file)."; 
   exit; 
}

my ( $name, $path, $extension ) = fileparse ( $filename, '..' ); 
$filename = $name . $extension;
$filename =~ tr/ /_/; 
$filename =~ s/[^$safe_filename_characters]//g;
if ( $filename =~ /^([$safe_filename_characters]+)$/ ) 
{ 
   $filename = $1; 
} 
else 
{ 
   die "Filename contains invalid characters"; 
}

if ($extension ne "ko") 
{
    error("Usted no ha ingresado un modulo correcto.");
} 

my $upload_filehandle = $query->upload("modulo");
open ( UPLOADFILE, ">$upload_dir/modulo.ko" ) or die "$!"; 
binmode UPLOADFILE;
while ( <$upload_filehandle> )
{
   print UPLOADFILE;
}

close UPLOADFILE;

print $query->header ( );
print "<p>'$filename' subido correctamente</p>";

sub error {
   print $query->header(),
         $query->start_html(-title=>'Error'),
         shift,
         $query->end_html;
   exit(0);
}