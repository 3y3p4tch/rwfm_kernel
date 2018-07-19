from django.db import models
from polymorphic import PolymorphicModel

# Create your models here.

class Id(PolymorphicModel):
    hostid = models.TextField()

class ObjId(Id):
    devid = models.TextField()
    inum = models.IntegerField()

    #class Meta:
    #    unique_together = ('hostid', 'devid', 'inum',)

    def __unicode__(self):
        return '%s %s %s' % (self.hostid, self.devid, self.inum)

class UserId(Id):
    uid = models.TextField()

    #class Meta:
    #    unique_together = ('hostid', 'uid', 'pid',)

    def __unicode__(self):
        return '%s %s' % (self.hostid, self.uid)

class SubId(Id):
    uid = models.TextField()
    pid = models.IntegerField()

    #class Meta:
    #    unique_together = ('hostid', 'uid', 'pid',)

    def __unicode__(self):
        return '%s %s %s' % (self.hostid, self.uid, self.pid)

class GroupId(Id):
    gid = models.TextField()
    members = models.ManyToManyField(UserId, related_name='user_groups', blank=True, null=True)

    #class Meta:
    #    unique_together = ('hostid', 'gid')

    def __unicode__(self):
        return '%s %s' % (self.hostid, self.gid)

class Object(models.Model):
    obj_id = models.ForeignKey(ObjId, related_name='+')
    admin = models.ForeignKey(UserId, related_name='+')
    readers = models.ManyToManyField(UserId, related_name='object_readers', blank=True, null=True)
    writers = models.ManyToManyField(UserId, related_name='object_writers', blank=True, null=True)

    class Meta:
        ordering = ('obj_id',)

class Subject(models.Model):
    sub_id = models.ForeignKey(SubId, related_name='+')
    admin = models.ForeignKey(UserId, related_name='+')
    readers = models.ManyToManyField(UserId, related_name='subject_readers', blank=True, null=True)
    writers = models.ManyToManyField(UserId, related_name='subject_writers', blank=True, null=True)

    class Meta:
        ordering = ('sub_id',)

class SockId(Id):
    uid = models.TextField()
    pid = models.IntegerField()
    fd = models.IntegerField()

    def __unicode__(self):
        return '%s %s %s %s' % (self.hostid, self.uid, self.pid, self.fd)

class Socket(models.Model):
    sock_id = models.ForeignKey(SockId, related_name='+')
    admin = models.ForeignKey(UserId, related_name='+')
    readers = models.ManyToManyField(UserId, related_name='socket_readers', blank=True, null=True)
    writers = models.ManyToManyField(UserId, related_name='socket_writers', blank=True, null=True)

    class Meta:
        ordering = ('sock_id',)

class Addr(models.Model):
    sock_id = models.ForeignKey(SockId, related_name='sock_id')
    ip = models.TextField()
    port = models.IntegerField()

    def __unicode__(self):
        return '%s %s' % (self.ip, self.port)

class Connection(models.Model):
    server = models.ForeignKey(SockId, related_name='server')
    client = models.ForeignKey(SockId, related_name='client')
    readers = models.ManyToManyField(UserId, related_name='connection_readers', blank=True, null=True)
    writers = models.ManyToManyField(UserId, related_name='connection_writers', blank=True, null=True)

#pipe data structures---------------------------------------------------------
class Pipe(models.Model):
	pipe_ref_num = models.IntegerField()
	admin = models.ForeignKey(UserId, related_name='+')
	readers = models.ManyToManyField(UserId, related_name='pipe_readers', blank=True, null=True)
	writers = models.ManyToManyField(UserId, related_name='pipe_writers', blank=True, null=True)


	def __unicode__(self):
		return '%d' % (self.pipe_ref_num)


class PipeId(Id):
	"""docstring for PipeId"""
	uid = models.TextField()
	pid = models.IntegerField()
	fd = models.IntegerField()
	pipe_ref_num = models.IntegerField()

	
	def __unicode__(self):
		return '%s %s %s %s' % (self.hostid, self.uid, self.pid, self.fd)	


class Fifo(Id):
    path = models.TextField()
    admin = models.ForeignKey(UserId, related_name='+')
    readers = models.ManyToManyField(UserId, related_name='fifo_readers', blank=True, null=True)
    writers = models.ManyToManyField(UserId, related_name='fifo_writers', blank=True, null=True)


    def __unicode__(self):
        return '%s %s' % (self.hostid, self.path)   


class FifoId(Id):
    uid = models.TextField()
    pid = models.IntegerField()
    fd = models.IntegerField()
    path = models.TextField()

    def __unicode__(self):
        return '%s %s %s %s %s' % (self.hostid, self.uid, self.pid, self.fd,self.path)  

#-----------------------------------------------------------------
