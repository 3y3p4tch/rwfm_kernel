#!/usr/bin/perl
#
# Lists members of all groups, or optionally just the group
# specified on the command line
#
# Copyright © 2010-2013 by Zed Pobre (zed@debian.org or zed@resonant.org)
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#

use strict; use warnings;

$ENV{"PATH"} = "/usr/bin:/bin";

#my $userorgroup = shift;
#my $wantedgroup = shift;
my $hostid = shift;

my %groupmembers;
my $usertext = `getent passwd`;

my @users = $usertext =~ /^([a-zA-Z0-9_-]+):/gm;
#my @users = $usertext =~ `cat /etc/passwd | awk -F: '{print \$3}'`;

foreach my $userid (@users)
{
    my $usergrouptext = `id -G $userid`;
    my $useridtoadd = `id -u $userid`;
    my $uidcmd = "./add_users_and_groups 0 $hostid $useridtoadd";
    `$uidcmd`;
    my @grouplist = split(' ',$usergrouptext);

    foreach my $group (@grouplist)
    {
        $groupmembers{$group}->{$useridtoadd} = 1;
    }
}

#if($wantedgroup)
#{
#    print_group_members($wantedgroup);
#}
#else
#{
foreach my $group (sort keys %groupmembers)
{
    my $grpcmd = "./add_users_and_groups 1 $hostid $group ";
    #print "Group ",$group," has the following members:\n";
    foreach my $member (sort keys %{$groupmembers{$group}})
    {
        #print $member;
        $member =~ s/^ *//;
        $member =~ s/ *$//;
        $member =~ s/\n*$//;
        $grpcmd .= "$member ";
    }
    $grpcmd =~ s/ *$//;
    $grpcmd .= "\n";
    `$grpcmd`;
}
#}

#sub print_group_members
#{
#    my ($group) = @_;
#    return unless $group;

#    foreach my $member (sort keys %{$groupmembers{$group}})
#    {
#        print $member;
#    }
#}
