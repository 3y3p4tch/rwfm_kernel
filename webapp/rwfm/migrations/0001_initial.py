# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('contenttypes', '0001_initial'),
    ]

    operations = [
        migrations.CreateModel(
            name='Addr',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('ip', models.TextField()),
                ('port', models.IntegerField()),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Connection',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Id',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('hostid', models.TextField()),
            ],
            options={
                'abstract': False,
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='GroupId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('gid', models.TextField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='FifoId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('uid', models.TextField()),
                ('pid', models.IntegerField()),
                ('fd', models.IntegerField()),
                ('path', models.TextField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='Fifo',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('path', models.TextField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='Object',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
            ],
            options={
                'ordering': ('obj_id',),
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='ObjId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('devid', models.TextField()),
                ('inum', models.IntegerField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='Pipe',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('pipe_ref_num', models.IntegerField()),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='PipeId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('uid', models.TextField()),
                ('pid', models.IntegerField()),
                ('fd', models.IntegerField()),
                ('pipe_ref_num', models.IntegerField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='Socket',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
            ],
            options={
                'ordering': ('sock_id',),
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='SockId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('uid', models.TextField()),
                ('pid', models.IntegerField()),
                ('fd', models.IntegerField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='SubId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('uid', models.TextField()),
                ('pid', models.IntegerField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.CreateModel(
            name='Subject',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
            ],
            options={
                'ordering': ('sub_id',),
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='UserId',
            fields=[
                ('id_ptr', models.OneToOneField(parent_link=True, auto_created=True, primary_key=True, serialize=False, to='rwfm.Id')),
                ('uid', models.TextField()),
            ],
            options={
                'abstract': False,
            },
            bases=('rwfm.id',),
        ),
        migrations.AddField(
            model_name='subject',
            name='admin',
            field=models.ForeignKey(related_name='+', to='rwfm.UserId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='subject',
            name='readers',
            field=models.ManyToManyField(related_name='subject_readers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='subject',
            name='sub_id',
            field=models.ForeignKey(related_name='+', to='rwfm.SubId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='subject',
            name='writers',
            field=models.ManyToManyField(related_name='subject_writers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='socket',
            name='admin',
            field=models.ForeignKey(related_name='+', to='rwfm.UserId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='socket',
            name='readers',
            field=models.ManyToManyField(related_name='socket_readers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='socket',
            name='sock_id',
            field=models.ForeignKey(related_name='+', to='rwfm.SockId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='socket',
            name='writers',
            field=models.ManyToManyField(related_name='socket_writers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pipe',
            name='admin',
            field=models.ForeignKey(related_name='+', to='rwfm.UserId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pipe',
            name='readers',
            field=models.ManyToManyField(related_name='pipe_readers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pipe',
            name='writers',
            field=models.ManyToManyField(related_name='pipe_writers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='object',
            name='admin',
            field=models.ForeignKey(related_name='+', to='rwfm.UserId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='object',
            name='obj_id',
            field=models.ForeignKey(related_name='+', to='rwfm.ObjId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='object',
            name='readers',
            field=models.ManyToManyField(related_name='object_readers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='object',
            name='writers',
            field=models.ManyToManyField(related_name='object_writers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='id',
            name='polymorphic_ctype',
            field=models.ForeignKey(related_name='polymorphic_rwfm.id_set', editable=False, to='contenttypes.ContentType', null=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='groupid',
            name='members',
            field=models.ManyToManyField(related_name='user_groups', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='fifo',
            name='admin',
            field=models.ForeignKey(related_name='+', to='rwfm.UserId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='fifo',
            name='readers',
            field=models.ManyToManyField(related_name='fifo_readers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='fifo',
            name='writers',
            field=models.ManyToManyField(related_name='fifo_writers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='connection',
            name='client',
            field=models.ForeignKey(related_name='client', to='rwfm.SockId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='connection',
            name='readers',
            field=models.ManyToManyField(related_name='connection_readers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='connection',
            name='server',
            field=models.ForeignKey(related_name='server', to='rwfm.SockId'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='connection',
            name='writers',
            field=models.ManyToManyField(related_name='connection_writers', null=True, to='rwfm.UserId', blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='addr',
            name='sock_id',
            field=models.ForeignKey(related_name='sock_id', to='rwfm.SockId'),
            preserve_default=True,
        ),
    ]
