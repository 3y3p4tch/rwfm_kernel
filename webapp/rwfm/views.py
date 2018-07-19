from django.shortcuts import render

# Create your views here.
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt

from rest_framework.renderers import JSONRenderer
from rest_framework.parsers import JSONParser
from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response

from rwfm.models import Id, SubId, ObjId, UserId, GroupId, SockId, Subject, Object, Socket, Addr, Connection, PipeId, Pipe
from rwfm.serializers import SubIdSerializer, ObjIdSerializer, UserIdSerializer, GroupIdSerializer, AddrSerializer, SockIdSerializer, PipeIdSerializer
import sets, json, ast

class JSONResponse(HttpResponse):
    """
    An HttpResponse that renders its content into JSON.
    """
    def __init__(self, data, **kwargs):
        content = JSONRenderer().render(data)
        kwargs['content_type'] = 'application/json'
        super(JSONResponse, self).__init__(content, **kwargs)

#@api_view(['GET'])
@csrf_exempt
def home(request):
    """
    Display welcome message.
    """
    data = "Readers Writers Flow Model demo."
    return JSONResponse(data)

@csrf_exempt
def reset(request):
    """
    Reset DB to null.
    """
    data = "DB set to NULL."
    for s in Subject.objects.all():
        s.delete()

    for sid in SubId.objects.all():
        sid.delete()

    for o in Object.objects.all():
        o.delete()

    for oid in ObjId.objects.all():
        oid.delete()

    return JSONResponse(data)

@api_view(['POST'])
def add_user(request):
    """
    Add a user
    Example post parameters -
        {
         "hostid": "localhost", "uid": "ataware"
        }
    """
    if request.method == 'POST':
        #create user ID
        serializer = UserIdSerializer(data=request.data)
        if serializer.is_valid():
            subjectid = serializer.save()
        else:
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

        return Response(request.data, status=status.HTTP_201_CREATED)

@api_view(['POST'])
def add_group(request):
    """
    Add a user
    Example post parameters -
        {
            "hostid": "localhost",
            "gid": "0",
            "members" : "0,1,2,55,100"
        }
    """
    if request.method == 'POST':
        #create user ID
        mydata =  request.data.copy()
        members = mydata.pop("members")
        serializer = GroupIdSerializer(data=mydata)
        if serializer.is_valid():
            group = serializer.save()
        else:
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

        #create and add member of this group
        user = {}
        for u in members.split(','):
            user["hostid"] = group.hostid
            user["uid"] = u.strip()

            serializer = UserIdSerializer(data=user)
            if serializer.is_valid():
                user_object = serializer.save()
                group.members.add(user_object)
            else:
                return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

        return Response(request.data, status=status.HTTP_201_CREATED)

@api_view(['POST'])
def subject_detail(request):
    """
    View subject details.

    Example POST parameters -

        {
        "hostid": "localhost", "uid": "ataware", "pid":111
        }

    """
    if request.method == 'POST':
        response={}
        subid = SubId.objects.get(hostid=request.data['hostid'], uid=request.data['uid'], pid=request.data['pid'])
        try:
            subject = Subject.objects.get(sub_id=subid)
        except:
            pass

        serializer = SubIdSerializer(subject.sub_id)
        response["sub_id"] = JSONRenderer().render(serializer.data)

        serializer = UserIdSerializer(subject.admin)
        response["admin"] = JSONRenderer().render(serializer.data)

        readers = subject.readers.all()
        serializer = UserIdSerializer(readers, many=True)
        response["readers"] = JSONRenderer().render(serializer.data)

        writers = subject.writers.all()
        serializer = UserIdSerializer(writers, many=True)
        response["writers"] = JSONRenderer().render(serializer.data)

        return Response(response, status=status.HTTP_201_CREATED)

@api_view(['POST'])
def delete_subject(request):
    """
    Create subject.
    Example POST parameters -

        {
        "hostid": "localhost", "uid": "ataware", "pid":111
        }

    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 0
        ret["errors"] = "None"
        try:
            subid = SubId.objects.get(hostid=request.data['hostid'], uid=request.data['uid'], pid=request.data['pid'])
        except:
            ret["errors"] = "SubID does not exist."
            return Response(ret, status=status.HTTP_200_OK)

        subject = Subject.objects.get(sub_id=subid)
        subject.delete()
        subid.delete()
        ret["status"] = 1
        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def subject_present(request):
    """
    Verify whether a subject is present.

    Example post parameters -

        { "hostid": "8323329", "uid": "1003", "pid":111}

    """
    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        mydata =  request.data.copy()
        try:
            subid = SubId.objects.get(hostid=mydata['hostid'], uid=mydata['uid'], pid=mydata['pid'])
        except:
            ret["errors"] = "Subject ID not found."
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        ret["status"] = 1
        ret["errors"] = "None."
        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def add_subject(request):
    """
    Add a subject entry for a process trying to open file for the first time.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329", "uid": "1003", "pid":111},
        "admin": { "hostid": "8323329",  "uid": "1003"}
        "parent_pid": 777
        }

    """
    if request.method == 'POST':
        ret = {}
        ret['status'] = 0
        sub_id = {}
        parent_sub_id = {}
        new_sock_id = {}
        #data = JSONParser().parse(request)
        mydata =  request.data.copy()
        print mydata

        #create subject ID
        if(type(mydata)==dict):
            sub_id = mydata["sub_id"]
            parent_sub_id = sub_id.copy()
            parent_sub_id["pid"] = mydata["parent_pid"]
        else:
            sub_id = ast.literal_eval(mydata.get("sub_id"))
            parent_sub_id = sub_id.copy()
            parent_sub_id["pid"] = ast.literal_eval(mydata.get("parent_pid"))

        serializer = SubIdSerializer(data=sub_id)
        if serializer.is_valid():
            subjectid = serializer.save()
        else:
            ret['errors'] = serializer.errors
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        #create admin user
        if(type(mydata)==dict):
            admin = mydata["admin"]
        else:
            admin = ast.literal_eval(mydata.get("admin"))

        serializer = UserIdSerializer(data=admin)
        if serializer.is_valid():
            adminuser = serializer.save()
        else:
            ret['errors'] = serializer.errors
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        subject = Subject(sub_id=subjectid, admin=adminuser)
        subject.save()

        #add all the users on this host as readers.
        for reader in UserId.objects.filter(hostid=subjectid.hostid):
            subject.readers.add(reader)

        #add admin as writer for this subject.
        subject.writers.add(adminuser)

        #if parent is not dummy add a new entry for each socket id and socket of parent process to a socket id and socket for child process
        if parent_sub_id["pid"]!=0:
            for sockid in SockId.objects.filter(pid=parent_sub_id["pid"]):
                new_sock_id["hostid"] = sockid.hostid
                new_sock_id["uid"] = sockid.uid
                new_sock_id["fd"] = sockid.fd
                new_sock_id["pid"] = sub_id["pid"]
                print "Adding duplicate socket for new process\n"
                print new_sock_id
                serializer = SockIdSerializer(data=new_sock_id)
                if serializer.is_valid():
                    created_sock_id = serializer.save()
                else:
                    ret['errors'] = serializer.errors
                    return Response(ret, status=status.HTTP_400_BAD_REQUEST)

                sock = Socket(sock_id=created_sock_id, admin=subject.admin)
                sock.save()

                #add readers of newely created socket
                for r in subject.readers.all():
                    sock.readers.add(r)

                #add writers of newely created socket
                for w in subject.writers.all():
                    sock.writers.add(w)
                sock.writers.add(subject.admin)
                sock.save()

            try:
                for connectionid in Connection.objects.all():
                    if connectionid.server.pid == parent_sub_id["pid"]:
                        new_server_sockid = SockId.objects.get(hostid=connectionid.server.hostid, uid=connectionid.server.uid, pid=sub_id["pid"], fd=connectionid.server.fd)
                        new_connection = Connection(server=new_server_sockid, client=connectionid.client)
                        new_connection.save()
                        print "Adding duplicate connection object for new server process\n"
                        for r in connectionid.readers.all():
                            new_connection.readers.add(r)
                        for w in connectionid.writers.all():
                            new_connection.writers.add(w)
                        new_connection.writers.add(subject.admin)
                        new_connection.save()

                for connectionid in Connection.objects.all():
                    if connectionid.client.pid == parent_sub_id["pid"]:
                        new_client_sockid = SockId.objects.get(hostid=connectionid.client.hostid, uid=connectionid.client.uid, pid=sub_id["pid"], fd=connectionid.client.fd)
                        new_connection = Connection(server=connectionid.server, client=new_client_sockid)
                        new_connection.save()
                        print "Adding duplicate connection object for new client process\n"
                        for r in connectionid.readers.all():
                            new_connection.readers.add(r)
                        for w in connectionid.writers.all():
                            new_connection.writers.add(w)
                        new_connection.writers.add(subject.admin)
                        new_connection.save()

            except Exception,e:
                print str(e)

            #pipe implementation yet to be done
            for pipeid in PipeId.objects.filter(pid=parent_sub_id["pid"]):
                new_pipe_id["hostid"] = pipeid.hostid
                new_pipe_id["uid"] = pipeid.uid
                new_pipe_id["fd"] = pipeid.fd
                new_pipe_id["pid"] = sub_id["pid"]
                print "Adding duplicate socket for new process\n"
                print new_pipe_id
                serializer = PipeIdSerializer(data=new_pipe_id)
                if serializer.is_valid():
                    created_pipe_id = serializer.save()
                else:
                    ret['errors'] = serializer.errors
                    return Response(ret, status=status.HTTP_400_BAD_REQUEST)

                pipe_id_read = PipeId(uid = pipe_data_read["uid"],pid = pipe_data_read["pid"],fd = pipe_data_read["fd"],pipe_ref_num=pipe_data_read["pipe_ref_num"])
                pipe = Pipe(pipe_ref_num = pipe_data_read["pipe_ref_num"],admin = subject.admin)
                sock.save()

                #add readers of newely created socket
                for r in subject.readers.all():
                    sock.readers.add(r)

                #add writers of newely created socket
                for w in subject.writers.all():
                    sock.writers.add(w)
                sock.writers.add(subject.admin)
                sock.save()    
        
            #end of pipe implemntation

        ret['status'] = 1
        ret['errors'] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

@api_view(['GET'])
def user_list(request):
    """
    List all subjects.
    """
    if request.method == 'GET':
        uids = UserId.objects.all()
        serializer = UserIdSerializer(uids, many=True)
        return Response(serializer.data)

@api_view(['GET'])
def group_list(request):
    """
    List all subjects.
    """
    if request.method == 'GET':
        gids = GroupId.objects.all()
        serializer = GroupIdSerializer(gids, many=True)
        return Response(serializer.data)

@api_view(['GET'])
def subject_list(request):
    """
    List all subjects.
    """
    if request.method == 'GET':
        sub_ids = SubId.objects.all()
        serializer = SubIdSerializer(sub_ids, many=True)
        return Response(serializer.data)

@api_view(['POST'])
def delete_object(request):
    """
    Delete an object.

    Example post parameters -
        {
        "hostid": "localhost", "devid": "2", "inum": 222
        }
    """
    if request.method == 'POST':
        ret = {}
        ret['status'] = 0
        try:
            ret['errors'] = "ObjectID not found"
            objid = ObjId.objects.get(hostid=request.data['hostid'], devid=request.data['devid'], inum=request.data['inum'])
            ret['errors'] = "Object not found"
            obj = Object.objects.get(obj_id=objid)
            ret['errors'] = "Failed to delete object."
            obj.delete()
            ret['errors'] = "Failed to delete objectID."
            objid.delete()
        except:
            return Response(ret, status=status.HTTP_404_NOT_FOUND)
        ret['status'] = 1
        ret['errors'] = "None"
        print "delete_object", ret, "\n"
        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def create_object(request):
    """
    Create a new object on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "localhost",  "uid": "ataware", "pid": 111 }, 
        "obj_id": { "hostid": "localhost", "devid": "3", "inum": 333 }
        }

    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        #data = JSONParser().parse(request)
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            o = mydata["obj_id"]
            s = mydata["sub_id"]
        else:
            o = ast.literal_eval(mydata.get("obj_id"))
            s = ast.literal_eval(mydata.get("sub_id"))

        #fetch subject ID
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            subject = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #create object ID
        serializer = ObjIdSerializer(data=o)
        if serializer.is_valid():
            objectid, created = serializer.save()
        else:
            ret["errors"] = serializer.errors;
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        obj = Object(obj_id=objectid, admin=subject.admin)
        obj.save()

        #add readers
        for r in subject.readers.all():
            obj.readers.add(r)

        #add writers
        for w in subject.writers.all():
            obj.writers.add(w)
        obj.writers.add(subject.admin)

        obj.save()
        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

@api_view(['GET'])
def object_list(request):
    """
    List all objects.
    """
    if request.method == 'GET':
        obj_ids = ObjId.objects.all()
        serializer = ObjIdSerializer(obj_ids, many=True)
        return Response(serializer.data)

@api_view(['POST'])
def object_detail(request):
    """
    View subject details.

    Example POST parameters -

        {
        "hostid": "localhost", "devid": "2", "inum":111
        }

    """
    if request.method == 'POST':
        response={}
        response["obj_id"] = request.data.copy()
        obj_id = ObjId.objects.get(hostid=request.data['hostid'], devid=request.data['devid'], inum=request.data['inum'])
        try:
            obj = Object.objects.get(obj_id=obj_id)
        except:
            pass

        serializer = UserIdSerializer(obj.admin)
        response["admin"] = JSONRenderer().render(serializer.data)

        readers = obj.readers.all()
        serializer = UserIdSerializer(readers, many=True)
        response["readers"] = JSONRenderer().render(serializer.data)

        writers = obj.writers.all()
        serializer = UserIdSerializer(writers, many=True)
        response["writers"] = JSONRenderer().render(serializer.data)

        return Response(response, status=status.HTTP_201_CREATED)


@api_view(['POST'])
def add_object(request):
    """
    Create an object entry for an existing file to be opened.

    Example post parameters -

        {
        "obj_id": { "hostid": "8323329", "devid": "2067", "inum": 21374 },
        "uid"   : "1003",
        "gid"   : "1005",
        "mode"  : "100644"
        }

    """
    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        ret["errors"] = ""
        #data = JSONParser().parse(request)
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            obj_id = mydata["obj_id"]
        else:
            obj_id = ast.literal_eval(mydata.get("obj_id"))

        hostid = obj_id["hostid"]
        uid  = mydata["uid"]
        gid  = mydata["gid"]
        mode = mydata["mode"]

        objectadmin = UserId.objects.get(hostid=hostid, uid=uid)

        readers=[]
        writers=[]
        group_members = GroupId.objects.get(hostid=objectadmin.hostid, gid=gid).members.all()
        world = UserId.objects.filter(hostid=objectadmin.hostid)

        #Handle world membership
        if int(mode[len(mode)-1]) in (4,6,7):
            readers = world

        if int(mode[len(mode)-1]) in (2,6,7):
            writers = world


        #Handle group members
        if int(mode[len(mode)-2]) in (4,5,6,7):
            readers = list(set(readers) | set(group_members))
        else:
            if readers:
                readers = list(set(readers) - set(group_members))

        if int(mode[len(mode)-2]) in (2,3,6,7):
            writers = list(set(writers) | set(group_members))
        else:
            if writers:
                writers = list(set(writers) - set(group_members))

        #Handle owner
        if int(mode[len(mode)-3]) in (4,5,6,7):
            rset = set(readers)
            rset.add(objectadmin)
            readers = list(rset)
        else:
            if objectadmin in readers:
                rset = set(readers)
                rset.remove(objectadmin)
                readers = list(rset)

        if int(mode[len(mode)-3]) in (2,3,6,7):
            wset = set(writers)
            wset.add(objectadmin)
            writers = list(wset)
        else:
            if objectadmin in writers:
                wset = set(writers)
                wset.remove(objectadmin)
                writers = list(wset)

        #create object ID
        #obj_id = mydata.pop("obj_id")

        serializer = ObjIdSerializer(data=obj_id)
        if serializer.is_valid():
            objectid, created = serializer.save()
        else:
            ret["errors"] = serializer.errors
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        if created:
            obj = Object(obj_id=objectid, admin=objectadmin)
            obj.save()

            for r in readers:
                obj.readers.add(r)

            for w in writers:
                obj.writers.add(w)

        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)


@api_view(['POST'])
def write_auth(request):
    """
    Authenticate whether subject 's' can write to object 'o'

    Example post parameters -

        {
        "sub_id": { "hostid": "localhost",  "uid": "ataware", "pid": 111 }, 
        "obj_id": { "hostid": "localhost", "devid": "2", "inum": 222 }
        }

    """

    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            o = mydata["obj_id"]
            s = mydata["sub_id"]
        else:
            o = ast.literal_eval(mydata.get("obj_id"))
            s = ast.literal_eval(mydata.get("sub_id"))

        #fetch object
        try:
            obj_id = ObjId.objects.get(hostid=o["hostid"], devid=o["devid"], inum=o["inum"])
            o = Object.objects.get(obj_id=obj_id)
        except:
            ret["errors"] = "Object not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch subject
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            s = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        sub_readers_set = set(s.readers.all())
        obj_readers_set = set(o.readers.all())

        sub_writers_set = set(s.writers.all())
        obj_writers_set = set(o.writers.all())

        if (s.admin in o.writers.all()) and sub_readers_set.issuperset(obj_readers_set) and sub_writers_set.issubset(obj_writers_set):
            ret["status"] = 1
            ret["errors"] = "None."
        else:
            ret["errors"] = "Denied by RWFM write rule."

        return Response(ret, status=status.HTTP_200_OK)


@api_view(['POST'])
def read_auth(request):
    """
    Authenticate whether subject 's' can read object 'o'

    Example post parameters -

        {
        "sub_id": { "hostid": "localhost",  "uid": "ataware", "pid": 111 }, 
        "obj_id": { "hostid": "localhost", "devid": "2", "inum": 222 }
        }

    """

    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        ret["errors"] = "None."
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            o = mydata["obj_id"]
            s = mydata["sub_id"]
        else:
            o = ast.literal_eval(mydata.get("obj_id"))
            s = ast.literal_eval(mydata.get("sub_id"))

        try:
            obj_id = ObjId.objects.get(hostid=o["hostid"], devid=o["devid"], inum=o["inum"])
            o = Object.objects.get(obj_id=obj_id)
        except:
            ret["errors"] = "Object not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch subject ID
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            s = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        if(s.admin in o.readers.all()):
            new_readers = list( set(s.readers.all()) & set(o.readers.all()) )
            new_writers = list( set(s.writers.all()) | set(o.writers.all()) )
            ret["status"] = 1
        else:
            ret["errors"] = "Denied by RWFM read rule."
            return Response(ret, status=status.HTTP_200_OK)

# Subject label is getting changed

        #remove existing readers.
        for r in s.readers.all():
            s.readers.remove(r)

        #add new readers.
        for r in new_readers:
            s.readers.add(r)

        #remove existing writers.
        for r in s.writers.all():
            s.writers.remove(r)

        #add new writers.
        for r in new_writers:
            s.writers.add(r)

        s.save()

        return Response(ret, status=status.HTTP_200_OK)


@api_view(['POST'])
def rdwr_auth(request):
    """
    Authenticate whether subject 's' can read and write object 'o'

    Example post parameters -

        {
        "sub_id": { "hostid": "localhost",  "uid": "ataware", "pid": 111 }, 
        "obj_id": { "hostid": "localhost", "devid": "2", "inum": 222 }
        }

    """

    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        ret["errors"] = "None."
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            o = mydata["obj_id"]
            s = mydata["sub_id"]
        else:
            o = ast.literal_eval(mydata.get("obj_id"))
            s = ast.literal_eval(mydata.get("sub_id"))

        try:
            obj_id = ObjId.objects.get(hostid=o["hostid"], devid=o["devid"], inum=o["inum"])
            o = Object.objects.get(obj_id=obj_id)
        except:
            ret["errors"] = "Object not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch subject ID
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            s = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        sub_readers_set = set(s.readers.all())
        obj_readers_set = set(o.readers.all())

        sub_writers_set = set(s.writers.all())
        obj_writers_set = set(o.writers.all())

        if s.admin in o.readers.all() and s.admin in o.writers.all() and sub_readers_set.issuperset(obj_readers_set) and sub_writers_set.issubset(obj_writers_set):
            ret["status"] = 1
        else:
            ret["errors"] = "Denied by RWFM read rule."
            return Response(ret, status=status.HTTP_200_OK)

# Subject label is getting changed

        #remove existing readers.
        for r in s.readers.all():
            s.readers.remove(r)

        #add new readers.
        for r in o.readers.all():
            s.readers.add(r)

        #remove existing writers.
        for r in s.writers.all():
            s.writers.remove(r)

        #add new writers.
        for r in o.writers.all():
            s.writers.add(r)

        s.save()

        return Response(ret, status=status.HTTP_200_OK)


@api_view(['POST'])
def downgrade_object(request):
    """
    Downgrade: Subject with label (s1,R1,W1) requests to downgrade an object with label (s2,R2,W2) to label (s3,R3,W3).

    IF 
    (s1=s2=s3) and (s1 is-in R2) and (R1=R2) and (W1=W2=W3) and (R3 superset-of R2) and [ (w2={s1}) OR (R3-R2 subset-of W2) ]
    THEN
        change the label of the object in the database
        ALLOW
    ELSE
        DENY


    Example post parameters -

    {
    "sub_id": { "hostid": "8323329", "uid": "1003", "pid":111},
    "obj_id": { "hostid": "8323329", "devid": "2067", "inum": 21374 },
    "new_label" : {
                        "admin" :  { "hostid": "8323329", "uid": "1003"},
                        "readers"  : [{"hostid":"ragnar","uid":"0"},{"hostid":"ragnar","uid":"1"},.......}],
                        "writers" : {}
              }
    }
    """
    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        ret["errors"] = "None."
        mydata = request.data.copy()

        if(type(mydata)==dict):
            s  = mydata["sub_id"]
            o = mydata["obj_id"]
            l = mydata["new_label"]
        else:
            s = ast.literal_eval(mydata.get("sub_id"))
            o = ast.literal_eval(mydata.get("obj_id"))
            l = ast.literal_eval(mydata.get("new_label"))

        #fetch obejct ID
        try:
            obj_id = ObjId.objects.get(hostid=o["hostid"], devid=o["devid"], inum=o["inum"])
            o = Object.objects.get(obj_id=obj_id)
        except:
            ret["errors"] = "Object not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch subject ID
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            s = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch admin object from new label.
        try:
            l_admin = UserId.objects.get(hostid=l["admin"]["hostid"], uid=l["admin"]["uid"])
        except:
            ret["errors"] = "Admin incorrect in label part."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch reader objects from new label.
        l_readers = []
        for u in l["readers"]:
            try:
                uobject = UserId.objects.get(hostid=u["hostid"], uid=u["uid"])
                l_readers.append(uobject)
            except:
                ret["errors"] = "Reader incorrect in label readers part."
                return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch writers objects from new label.
        l_writers = []
        for w in l["writers"]:
            try:
                wobject = UserId.objects.get(hostid=w["hostid"], uid=w["uid"])
                l_writers.append(wobject)
            except:
                ret["errors"] = "Writer incorrect in label writer part."
                return Response(ret, status=status.HTTP_404_NOT_FOUND)

        new_readers = set(l_readers)-set(o.readers.all())
        if s.admin==o.admin==l_admin and \
            s.admin in o.readers.all() and \
            set(s.readers.all())==set(o.readers.all()) and \
            set(s.writers.all())==set(o.writers.all())==set(l_writers) \
            and set(l_readers).issuperset(set(o.readers.all())) \
            and ( set(o.writers.all())==set([s.admin]) or new_readers.issubset(set(o.writers.all())) ) :
            print "\n Allow"
            o.admin = l_admin
            o.readers = l_readers
            o.writers = l_writers
            o.save()
            ret['status'] = 1
        else:
            ret['status'] = 0
            print "\n Deny"

        ret['errors'] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

@api_view(['POST'])
def upgrade_object(request):
    """
    Upgrade: Subject with label (s1,R1,W1) requests to upgrade an object with label (s2,R2,W2) to label (s3,R3,W3).

    Example post parameters -

    {
    "sub_id": { "hostid": "8323329", "uid": "1003", "pid":111},
    "obj_id": { "hostid": "8323329", "devid": "2067", "inum": 21374 },
    "new_label" : {
                        "admin" :  { "hostid": "8323329", "uid": "1003"},
                        "readers"  : [{"hostid":"ragnar","uid":"0"},{"hostid":"ragnar","uid":"1"},.......}],
                        "writers" : {}
              }
    }
    """
    if request.method == 'POST':
        ret={}
        ret["status"] = 0
        ret["errors"] = "None."
        mydata = request.data.copy()

        if(type(mydata)==dict):
            s  = mydata["sub_id"]
            o = mydata["obj_id"]
            l = mydata["new_label"]
        else:
            s = ast.literal_eval(mydata.get("sub_id"))
            o = ast.literal_eval(mydata.get("obj_id"))
            l = ast.literal_eval(mydata.get("new_label"))

        #fetch obejct ID
        try:
            obj_id = ObjId.objects.get(hostid=o["hostid"], devid=o["devid"], inum=o["inum"])
            o = Object.objects.get(obj_id=obj_id)
        except:
            ret["errors"] = "Object not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch subject ID
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            s = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch admin object from new label.
        try:
            l_admin = UserId.objects.get(hostid=l["admin"]["hostid"], uid=l["admin"]["uid"])
        except:
            ret["errors"] = "Admin incorrect in label part."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch reader objects from new label.
        l_readers = []
        for u in l["readers"]:
            try:
                uobject = UserId.objects.get(hostid=u["hostid"], uid=u["uid"])
                l_readers.append(uobject)
            except:
                ret["errors"] = "Reader incorrect in label readers part."
                return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #fetch writers objects from new label.
        l_writers = []
        for w in l["writers"]:
            try:
                wobject = UserId.objects.get(hostid=w["hostid"], uid=w["uid"])
                l_writers.append(wobject)
            except:
                ret["errors"] = "Writer incorrect in label writer part."
                return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #(s1=s2=s3) and (s1 is-in R2) and (R1=R2) and (W1=W2=W3) and (R3 subset-of R2)
        if s.admin==o.admin==l_admin and \
            s.admin in o.readers.all() and \
            set(s.readers.all())==set(o.readers.all()) and \
            set(s.writers.all())==set(o.writers.all())==set(l_writers) and \
            set(l_readers).issubset(set(o.readers.all())) :
            print "\n Allow"
            o.admin = l_admin
            o.readers = l_readers
            o.writers = l_writers
            o.save()
            ret['status'] = 1
        else:
            ret['status'] = 0
            print "\n Deny"

        ret['errors'] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

#===================================== Socket specific APIs ===============================================
@api_view(['POST'])
def delete_socket(request):
    """
    Delete a socket.

    Example post parameters -
        {
        "hostid": "localhost", "uid": "2", "pid": 222, "fd" : 10
        }
    """
    if request.method == 'POST':
        ret = {}
        ret['status'] = 0
        try:
            ret['errors'] = "SocketID not found"
            sockid = SockId.objects.get(hostid=request.data['hostid'], 
                    uid=request.data['uid'], pid=request.data['pid'], fd=request.data['fd'])
            ret['errors'] = "Socket not found"
            sock = Socket.objects.get(sock_id=sockid)
            ret['errors'] = "Failed to delete socket."
            sock.delete()
            ret['errors'] = "Failed to delete socketID."
            sockid.delete()
        except:
            return Response(ret, status=status.HTTP_404_NOT_FOUND)
        ret['status'] = 1
        ret['errors'] = "None"
        print "delete_socket", ret, "\n"
        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def create_socket(request):
    """
    Create a new socket on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "localhost",  "uid": "ataware", "pid": 111 }, 
        "fd" : 10 ,
        }

    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        #data = JSONParser().parse(request)
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            s = mydata["sub_id"]
            fd = mydata["fd"]
        else:
            s = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))

        #fetch subject ID
        try:
            sub_id = SubId.objects.get(hostid=s["hostid"], uid=s["uid"], pid=s["pid"])
            subject = Subject.objects.get(sub_id=sub_id)
        except:
            ret["errors"] = "Subject not found."
            print ret["errors"]
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #create socket ID
        sock_data = s
        sock_data["fd"] = fd
        serializer = SockIdSerializer(data=sock_data)
        if serializer.is_valid():
            socketid = serializer.save()
        else:
            ret["errors"] = serializer.errors;
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        sock = Socket(sock_id=socketid, admin=subject.admin)
        sock.save()

        #add readers
        for r in subject.readers.all():
            sock.readers.add(r)

        #add writers
        for w in subject.writers.all():
            sock.writers.add(w)
        sock.writers.add(subject.admin)

        sock.save()
        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

@api_view(['GET'])
def socket_list(request):
    """
    List all sockets.
    """
    if request.method == 'GET':
        sock_ids = SockId.objects.all()
        serializer = SockIdSerializer(sock_ids, many=True)
        return Response(serializer.data)

@api_view(['POST'])
def socket_detail(request):
    """
    View socket details.

    Example POST parameters -

        {
        "hostid": "localhost", "uid": "2", "pid":111, "fd" : 10
        }

    """
    if request.method == 'POST':
        response={}
        response["sock_id"] = request.data.copy()
        sock_id = SockId.objects.get(hostid=request.data['hostid'],
                uid=request.data['uid'], pid=request.data['pid'], fd=request.data['fd'])
        sock = {}

        print "Found:"
        print sock_id

        try:
            sock = Socket.objects.get(sock_id=sock_id)
            print "Socket:"
            print sock
        except:
            print "Exception"
            pass

        serializer = UserIdSerializer(sock.admin)
        response["admin"] = JSONRenderer().render(serializer.data)

        readers = sock.readers.all()
        serializer = UserIdSerializer(readers, many=True)
        response["readers"] = JSONRenderer().render(serializer.data)

        writers = sock.writers.all()
        serializer = UserIdSerializer(writers, many=True)
        response["writers"] = JSONRenderer().render(serializer.data)

        return Response(response, status=status.HTTP_201_CREATED)

def downgrade_socket(s, sc, new_reader):
    "subject, socket, reader"
    subid_of_socket = SubId.objects.get(hostid=sc.sock_id.hostid, uid=sc.sock_id.uid, pid=sc.sock_id.pid)

    if subid_of_socket != s.sub_id:
        print "\ndowngrade_socket : Subject Id is not related to socket."
        return False

    if set(s.readers.all())!=set(sc.readers.all()) or set(s.writers.all())!=set(sc.writers.all()):
        print "\ndowngrade_socket : Downgrade not allowed."
        return False

    if set([s.admin]) == set(sc.writers.all()) or new_reader in sc.writers.all():
        sc.readers.add(new_reader)
        sc.save()
        return True

    print "\ndowngrade_socket : Downgrade not allowed."
    return False


@api_view(['POST'])
def delete_address(request):
    """
    Delete an address.

    Example post parameters -
        {
        "sock_id" : { "hostid": "localhost", "uid": "1002", "pid": 101, "fd": 22 },
        "ip": "127.0.0.1",
        "port": 2001
        }
    """
    if request.method == 'POST':
        ret = {}
        ret['status'] = 0
        s = request.data.pop('sock_id')
        ip = request.data.pop('ip')
        port = request.data.pop('port')
        try:
            ret['errors'] = "SockID not found"
            sock_id = SockId.objects.get(hostid=s['hostid'], uid=s['uid'], pid=s['pid'], fd=s['fd'])
            ret['errors'] = "Address not found"
            addr = Addr.objects.get(sock_id=sock_id, ip=ip, port=port)
            ret['errors'] = "Failed to delete Address."
            addr.delete()
        except:
            return Response(ret, status=status.HTTP_404_NOT_FOUND)
        ret['status'] = 1
        ret['errors'] = "None"
        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def create_address(request):
    """
    Create a new address.

    Example post parameters -
        {
        "sock_id" : { "hostid": "localhost", "uid": "1002", "pid": 101, "fd": 22 },
        "ip": "127.0.0.1",
        "port": 2001
        }

    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        #create socket ID
        serializer = AddrSerializer(data=request.data)
        if serializer.is_valid():
            address = serializer.save()
        else:
            ret["errors"] = serializer.errors;
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

@api_view(['GET'])
def address_list(request):
    """
    List all addresses.
    """
    if request.method == 'GET':
        addresses = Addr.objects.all()
        serializer = AddrSerializer(addresses, many=True)
        return Response(serializer.data)

@api_view(['POST'])
def bind(request):
    """
        {
           "sock_id" : { "hostid": "localhost", "uid": "2", "pid": 222, "fd" : 10},
           "ip" : "127.0.0.1",
           "port" : 1001
        }

    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        #data = JSONParser().parse(request)
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            s = mydata["sock_id"]
            ip = mydata["ip"]
            port = mydata["port"]
        else:
            s = ast.literal_eval(mydata.get("sock_id"))
            ip = mydata.get("ip")
            port = ast.literal_eval(mydata.get("port"))

        #fetch Socket
        try:
            ret['errors'] = "SockID not found"
            sock_id = SockId.objects.get(hostid=s['hostid'], uid=s['uid'], pid=s['pid'], fd=s['fd'])
            ret["errors"] = "Socket not found."
            socket = Socket.objects.get(sock_id=sock_id)
            ret["errors"] = "Subject ID for server not found."
            serverid = SubId.objects.get(hostid=s['hostid'], uid=s['uid'], pid=s['pid'])
            ret["errors"] = "Server not found."
            server = Subject.objects.get(sub_id=serverid)
            ret["errors"] = "None."
        except:
            ret["status"] = 0
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #create address
        addr_data = {}

        addr_data["sock_id"] = s
        addr_data["ip"] = ip
        addr_data["port"] = port

        serializer = AddrSerializer(data=addr_data)
        if serializer.is_valid():
            addr = serializer.save()
        else:
            ret["errors"] = serializer.errors;
            ret["status"] = 0
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)

        socket.readers = list( set(socket.readers.all()) & set(server.readers.all()) )
        socket.writers = list( set(socket.writers.all()) | set(server.writers.all()) )

        socket.save()

        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def connect(request):
    """
        Specify socketid of client and (ip, port) of server.
        {
           "sock_id" : { "hostid": "localhost", "uid": "2", "pid": 222, "fd" : 20},
           "server_ip" : "127.0.0.1",
           "port" : 1001
        }
    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        #data = JSONParser().parse(request)
        mydata =  request.data.copy()

        if(type(mydata)==dict):
            c = mydata["sock_id"]
            server_ip = mydata["server_ip"]
            port = mydata["port"]
        else:
            c = ast.literal_eval(mydata.get("sock_id"))
            server_ip = mydata.get("server_ip")
            port = ast.literal_eval(mydata.get("port"))

        #fetch Socket
        try:
            ret['errors'] = "client not found"
            c_subid = SubId.objects.get(hostid=c['hostid'], uid=c['uid'], pid=c['pid'])
            client = Subject.objects.get(sub_id=c_subid)

            ret['errors'] = "SockID not found"
            c_sock_id = SockId.objects.get(hostid=c['hostid'], uid=c['uid'], pid=c['pid'], fd=c['fd'])

            ret["errors"] = "Socket not found."
            client_sock = Socket.objects.get(sock_id=c_sock_id)

            ret["errors"] = "None."
        except:
            ret["status"] = 0
            print ret["errors"]
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        try:
            server_address = Addr.objects.get(ip=server_ip, port=port)
            server_sock = Socket.objects.get(sock_id=server_address.sock_id)
            s_subid = SubId.objects.get(hostid=server_sock.sock_id.hostid, 
                        uid=server_sock.sock_id.uid, pid=server_sock.sock_id.pid)
            server = Subject.objects.get(sub_id=s_subid)
        except:
            ret["status"] = 0
            ret["errors"] = "No server information found."
            print ret["errors"]
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        client_sock.readers = list( set(client_sock.readers.all()) & set(client.readers.all()) )
        client_sock.writers = list( set(client_sock.writers.all()) | set(client.writers.all()) )
        client_sock.save()

# Subject label is getting changed

        if client.admin in server_sock.readers.all():
            client.readers = list(set(client.readers.all()) & set(server_sock.readers.all()))
            client.writers = list(set(client.writers.all()) | set(server_sock.writers.all()))
            client.save()
            client_sock.readers = list( set(client_sock.readers.all()) & set(client.readers.all()) )
            client_sock.writers = list( set(client_sock.writers.all()) | set(client.writers.all()) )
            client_sock.save()
        else:
            ret["status"] = 0
            ret["errors"] = "Client admin missing from server socket readers."
            print ret["errors"]
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        if server.admin in client_sock.readers.all() or downgrade_socket(client, client_sock, server_sock.admin):
            connection = Connection(server=server_sock.sock_id, client=c_sock_id)
            connection.save()
            for r in client.readers.all():
                connection.readers.add(r)
            for w in client.writers.all():
                connection.writers.add(w)
            connection.save()
        else:
            ret["status"] = 0
            ret["errors"] = "Server admin missing from client socket reader or downgrade failed."
            print ret["errors"]
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def accept(request):
    """
        Specify old and new socket ids.

        {
           "old_sockid" : { "hostid": "localhost", "uid": "1000", "pid": 7525, "fd" : 10},
           "new_sockfd" : 11
        }

    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        mydata = request.data.copy()
        o = ast.literal_eval(mydata.get("old_sockid"))
        new_sockfd = mydata.get("new_sockfd")

        #fetch Socket
        try:
            ret['errors'] = "Old Socket not found"
            old_sockid = SockId.objects.get(hostid=o['hostid'], uid=o['uid'], pid=o['pid'], fd=o['fd'])
            old_sock = Socket.objects.get(sock_id=old_sockid)

            ret['errors'] = "Server not found"
            s_subid = SubId.objects.get(hostid=o['hostid'], uid=o['uid'], pid=o['pid'])
            server = Subject.objects.get(sub_id=s_subid)

            ret["errors"] = "Connection not found."
            connection = Connection.objects.get(server=old_sockid)

            ret['errors'] = "New SockID not found"
            new_sockdata = {}
            new_sockdata['hostid'] = o['hostid']
            new_sockdata['uid'] = o['uid']
            new_sockdata['pid'] = o['pid']
            new_sockdata['fd'] = new_sockfd

            serializer = SockIdSerializer(data=new_sockdata)
            if serializer.is_valid():
                new_sockid = serializer.save()
            else:
                ret["errors"] = serializer.errors;
                return Response(ret, status=status.HTTP_400_BAD_REQUEST)
            ret["errors"] = "None"
        except:
            ret["status"] = 0
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

# Subject label is getting changed

        server.readers = list( set(server.readers.all()) & set(connection.readers.all()) )
        server.writers = list( set(server.writers.all()) | set(connection.writers.all()) )
        server.save()

        old_sock.readers = list( set(old_sock.readers.all()) & set(server.readers.all()) )
        old_sock.writers = list( set(old_sock.writers.all()) | set(server.writers.all()) )
        old_sock.save()

        #create new socket, delete old one if exist.

#        for sins_of_past in Socket.objects.all():
#            if sins_of_past.sock_id == new_sockid:
#                sins_of_past.delete()

        new_sock = Socket(sock_id=new_sockid, admin=old_sock.admin)
        new_sock.save()
        for r in old_sock.readers.all():
            new_sock.readers.add(r)

        for w in old_sock.writers.all():
            new_sock.writers.add(w)
        new_sock.save()

        client_sock = Socket.objects.get(sock_id=connection.client)

        if client_sock.admin in new_sock.readers.all() or downgrade_socket(server, new_sock, client_sock.admin):
            connection.server = new_sockid
            connection.readers = list( set(connection.readers.all()) & set(server.readers.all()) )
            connection.writers = list( set(connection.writers.all()) | set(server.writers.all()) )
            connection.save()
        else:
            ret["status"] = 0
            ret["errors"] = "Client admin missing from new server socket reader or downgrade failed."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def send(request):
    """
        { 
        "hostid": "localhost",
        "uid": "1000",
        "pid": 7525,
        "fd" : 10
        }
    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        s =  request.data.copy()
        print s

        #fetch Socket
        try:
            ret['errors'] = "Socket not found"
            s_sockid = SockId.objects.get(hostid=s['hostid'], uid=s['uid'], pid=s['pid'], fd=s['fd'])
            print s_sockid
            s_socket = Socket.objects.get(sock_id=s_sockid)
            print s_socket

            ret['errors'] = "Sender not found"
            s_subid = SubId.objects.get(hostid=s['hostid'], uid=s['uid'], pid=s['pid'])
            sender = Subject.objects.get(sub_id=s_subid)

            ret["errors"] = "None"
        except Exception, e:
            print ret["errors"]
            print str(e)
            ret["status"] = 0
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        connection = None
        try:
            connection = Connection.objects.get(server=s_sockid)
            r_sockid = connection.client
        except:
            pass

        try:
            connection = Connection.objects.get(client=s_sockid)
            r_sockid = connection.server
        except:
            pass

        if connection:
            try:
                r_socket = Socket.objects.get(sock_id=r_sockid)
                r_subid = SubId.objects.get(hostid=r_sockid.hostid, uid=r_sockid.uid, pid=r_sockid.pid)
                receiver = Subject.objects.get(sub_id=r_subid)
            except:
                ret["errors"] = "Receiver not found."
                print ret["errors"]
                ret["status"] = 0
                return Response(ret, status=status.HTTP_404_NOT_FOUND)
        else:
            ret["errors"] = "Connection not found."
            print ret["errors"]
            ret["status"] = 0
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        #update readers, writers of sender socket.
        s_socket.readers = list( set(s_socket.readers.all()) & set(sender.readers.all()) )
        s_socket.writers = list( set(s_socket.writers.all()) | set(sender.writers.all()) )
        s_socket.save()

        if r_socket.admin in s_socket.readers.all() or downgrade_socket(sender, s_socket, r_socket.admin):
            connection.readers = list( set(connection.readers.all()) & set(s_socket.readers.all()) )
            connection.writers = list( set(connection.writers.all()) | set(s_socket.writers.all()) )
            connection.save()
            r_socket.readers = list( set(r_socket.readers.all()) & set(connection.readers.all()) )
            r_socket.writers = list( set(r_socket.writers.all()) | set(connection.writers.all()) )
            r_socket.save()
        else:
            ret["status"] = 0
            ret["errors"] = "Receiver's admin missing from sender socket reader or downgrade failed."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

        return Response(ret, status=status.HTTP_200_OK)

@api_view(['POST'])
def receive(request):
    """
        {
        "hostid": "localhost",
        "uid": "1000",
        "pid": 7530,
        "fd" : 20
        }
    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        r =  request.data.copy()

        #fetch Socket
        try:
            ret['errors'] = "Receiver socket not found"
            r_sockid = SockId.objects.get(hostid=r['hostid'], uid=r['uid'], pid=r['pid'], fd=r['fd'])
            r_socket = Socket.objects.get(sock_id=r_sockid)

            ret['errors'] = "Receiver not found."
            r_subid = SubId.objects.get(hostid=r['hostid'], uid=r['uid'], pid=r['pid'])
            receiver = Subject.objects.get(sub_id=r_subid)

            ret["errors"] = "None"
        except:
            ret["status"] = 0
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

# Subject label is getting changed

        #update readers, writers of receiver and receiver socket.
        receiver.readers = list( set(receiver.readers.all()) & set(r_socket.readers.all()) )
        receiver.writers = list( set(receiver.writers.all()) | set(r_socket.writers.all()) )
        receiver.save()

        r_socket.readers = list( set(r_socket.readers.all()) & set(receiver.readers.all()) )
        r_socket.writers = list( set(r_socket.writers.all()) | set(receiver.writers.all()) )
        r_socket.save()

        return Response(ret, status=status.HTTP_200_OK)


@api_view(['GET'])
def connection_list(request):
    """
    List all connections.
    """
    if request.method == 'GET':
        connections = Connection.objects.all()
        print connections, "\n"
        #serializer = ObjIdSerializer(obj_ids, many=True)
        return Response(serializer.data)

@api_view(['POST'])
def close(request):
    """
        {
        "sub_id": { "hostid": "8323329", "uid": "1003", "pid":111},
        "fd" : 20
        }
    """
    if request.method == 'POST':
        ret = {}
        ret["status"] = 1
        mydata =  request.data.copy()
        sockid = {}

        if(type(mydata)==dict):
            sockid = mydata["sub_id"]
            sockid["fd"] = mydata["fd"]
        else:
            sockid = ast.literal_eval(mydata.get("sub_id"))
            sockid["fd"] = ast.literal_eval(mydata.get("fd"))

        try:
            print sockid
            sock_id = SockId.objects.get(hostid=sockid['hostid'], uid=sockid['uid'], pid=sockid['pid'], fd=sockid['fd'])

            connection = None
            other_sock_id = None
        except Exception,e:
            pass

        try:
            connection = Connection.objects.get(server=sock_id)
            other_sock_id = connection.client
            connection.server = 0
            connection.save()
        except:
            pass

        try:
            connection = Connection.objects.get(client=sock_id)
            other_sock_id = connection.server
            connection.client = 0
            connection.save()
        except:
            pass

        try:
            addr = Addr.objects.get(sock_id=sock_id)
            addr.delete()
        except:
            pass

        try:
            if connection and other_sock_id==0:
                connection.delete()

            sock = Socket.objects.get(sock_id=sock_id)
            sock.delete()
            sock_id.delete()
        except Exception,e:
            pass

        return Response(ret, status=status.HTTP_200_OK)


#pipe related calls-----------------------
pipe_reference_num = 0
@api_view(['POST'])
def create_pipe(request):
    """
    Create a new pipe on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "fd" : {"fdw": 123 , "fdr":456} 
        
        }

    """

    print "Test-------------"
    if request.method == 'POST':

        print "in post if"
        ret = {}
        ret["status"] = 1
        print request.data
        mydata = request.data.copy()
        print mydata
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            fd = mydata["fd"]
            
           

        else:
            print "in else"
            sub_id = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))
           
        
        pipe_ref_num += 1

        try:
            subject_id = SubId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"])
            subject = Subject.objects.get(sub_id=subject_id)

        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)  
        #pipeId for fdr---------------------

        pipe_data_read = sub_id
        pipe_data_read["fd"] = fd["fdr"]
        
        pipe_data_read["pipe_ref_num"] = pipe_ref_num
        print pipe_data_read
        serializer = PipeIdSerializer(data=pipe_data_read)
        if serializer.is_valid():
            print "in ser if"
            pipeid = serializer.save()
            print "pipeid",pipeid
        else:
            print "in ser else"
            ret["errors"] = serializer.errors;
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)
        print "after ser"
        pipe_id_read = PipeId(uid = pipe_data_read["uid"],pid = pipe_data_read["pid"],fd = pipe_data_read["fd"],pipe_ref_num=pipe_data_read["pipe_ref_num"])

       
        #pipeId for fdw------------------------------

        pipe_data_write = sub_id
        pipe_data_write["fd"] = fd["fdw"]
        
        pipe_data_write["pipe_ref_num"] = pipe_ref_num
        print pipe_data_write
        serializer = PipeIdSerializer(data=pipe_data_write)
        if serializer.is_valid():
            print "in ser if"
            pipeid = serializer.save()
            print "pipeid",pipeid
        else:
            print "in ser else"
            ret["errors"] = serializer.errors;
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)
        print "after ser"
        pipe_id_write = PipeId(uid = pipe_data_write["uid"],pid = pipe_data_write["pid"],fd = pipe_data_write["fd"],pipe_ref_num=pipe_data_write["pipe_ref_num"])

        pipe = Pipe(pipe_ref_num = pipe_data_read["pipe_ref_num"],admin = subject.admin)
        pipe.save()

        for r in subject.readers.all():
            pipe.readers.add(r)

        #add writers
        for w in subject.writers.all():
            pipe.writers.add(w)
        pipe.writers.add(subject.admin)

        pipe.save()
        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

@api_view(['POST'])
def pipe_read(request):
    """
    read pipe on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "fd" : 123
        }

    """
    if request.method == 'POST':

        ret = {}
        ret["status"] = 1
        mydata = request.data.copy()
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            fd = mydata["fd"]
        else:

            sub_id = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))


        try:
            pipe_id = PipeId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"],fd=fd )
            print "pipe_id",pipe_id.pipe_ref_num
            pipe = Pipe.objects.get(pipe_ref_num = pipe_id.pipe_ref_num)
        except:
            ret["errors"] = "PipeId or pipe not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)  


        try:
            subject_id = SubId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"])
            subject = Subject.objects.get(sub_id=subject_id)

        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

# Subject label is getting changed

        if subject.admin in pipe.readers.all():
            subject.readers = list(set(pipe.readers.all()) & set(subject.readers.all()) )

            subject.writers = list( set(pipe.writers.all()) | set(subject.writers.all()) )
            subject.save()

        else:
            ret["status"] = 0
            ret["errors"] = "subject missing from pipe's reader list"
            return Response(ret, status=status.HTTP_404_NOT_FOUND)



        return Response(ret, status=status.HTTP_200_OK)




@api_view(['POST'])
def pipe_write(request):
    """
    write pipe on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "fd" : 123
        }

    """
    if request.method == 'POST':

        ret = {}
        ret["status"] = 1
        mydata = request.data.copy()
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            fd = mydata["fd"]
        else:

            sub_id = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))


        try:
            pipe_id = PipeId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"],fd=fd )
            print "pipe_id",pipe_id.pipe_ref_num
            pipe = Pipe.objects.get(pipe_ref_num = pipe_id.pipe_ref_num)
        except:
            ret["errors"] = "PipeId or pipe not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)  


        try:
            subject_id = SubId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"])
            subject = Subject.objects.get(sub_id=subject_id)

        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

# Subject label is getting changed

        if subject.admin in pipe.writers.all():
            subject.readers = list(set(pipe.readers.all()) & set(subject.readers.all()) )

            subject.writers = list( set(pipe.writers.all()) | set(subject.writers.all()) )
            subject.save()

        else:
            ret["status"] = 0
            ret["errors"] = "subject missing from pipe's writers list"
            return Response(ret, status=status.HTTP_404_NOT_FOUND)



        return Response(ret, status=status.HTTP_200_OK)



@api_view(['POST'])
def create_pipe_id(request):
    """
    Create a new pipe on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "fd" : 123 ,
        "pipe_ref_num" : 12345 
        }

    """

    print "Test-------------"
    if request.method == 'POST':

        print "in post if"
        ret = {}
        ret["status"] = 1
        print request.data
        mydata = request.data.copy()
        print mydata
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            fd = mydata["fd"]
            
            pipe_ref_num = mydata["pipe_ref_num"]

        else:
            print "in else"
            sub_id = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))
            pipe_ref_num = ast.literal_eval(mydata.get("pipe_ref_num"))

        

        pipe_data = sub_id
        pipe_data["fd"] = fd
        
        pipe_data["pipe_ref_num"] = pipe_ref_num
        print pipe_data
        serializer = PipeIdSerializer(data=pipe_data)
        if serializer.is_valid():
            print "in ser if"
            pipeid = serializer.save()
            print "pipeid",pipeid
        else:
            print "in ser else"
            ret["errors"] = serializer.errors;
            return Response(ret, status=status.HTTP_400_BAD_REQUEST)
        print "after ser"
        pipe_id = PipeId(uid = pipe_data["uid"],pid = pipe_data["pid"],fd = pipe_data["fd"],pipe_ref_num=pipe_data["pipe_ref_num"])

        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)

       
   


        
@api_view(['POST'])       
def make_fifo(request):
    """
    Create a new pipe on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "path" : "myfile.c" 
        }

    """
    if request.method == 'POST':

        
        ret = {}
        ret["status"] = 1
        print request.data
        mydata = request.data.copy()
        print mydata
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            path = mydata["path"]
            
            

        else:
            
            sub_id = ast.literal_eval(mydata.get("sub_id"))
            path = ast.literal_eval(mydata.get("path"))


        try:
            subject_id = SubId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"])
            subject = Subject.objects.get(sub_id=subject_id)

        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)  
            
        

        

        
        fifo = Fifo(path = path,admin = subject.admin)
        fifo.save()

        for r in subject.readers.all():
            fifo.readers.add(r)

        #add writers
        for w in subject.writers.all():
            fifo.writers.add(w)
        fifo.writers.add(subject.admin)

        fifo.save()
        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)




@api_view(['POST'])       
def open(request):
    """
    open on behalf of a subject.

    Example post parameters -

        {
        "hostid": "8323329",  
        "uid": "1003", 
        "pid": 111, 
        "fd":123,
        "path" : "myfile.c" 
        }

    """
    if request.method == 'POST':

        
        ret = {}
        ret["status"] = 1
        print request.data
        mydata = request.data.copy()
        print mydata
        if(type(mydata) == dict):
            host_id = mydata["hostid"]
            u_id = mydata["uid"]
            p_id = mydata["pid"]
            fd = mydata["fd"]
            path = mydata["path"]
            
            

        else:
            
            host_id = ast.literal_eval(mydata.get("hostid"))
            u_id = ast.literal_eval(mydata.get("uid"))
            p_id = ast.literal_eval(mydata.get("pid"))
            fd = ast.literal_eval(mydata.get("fd"))
            path = ast.literal_eval(mydata.get("path"))


        
        

        

        
        fifo_id = FifoId(hostid=host_id,uid=u_id,pid=p_id,fd=fd,path = path)
        fifo_id.save()

        
        ret["status"] = 1
        ret["errors"] = "None"
        return Response(ret, status=status.HTTP_201_CREATED)




@api_view(['POST'])
def read_fifo(request):
    """
    read fifo on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "fd" : 123
        }

    """
    if request.method == 'POST':

        ret = {}
        ret["status"] = 1
        mydata = request.data.copy()
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            fd = mydata["fd"]
        else:

            sub_id = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))



        try:
            fifo_id = FifoId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"],fd=fd )
            #print "fifo_id",pipe_id.pipe_ref_num
            fifo = Fifo.objects.get(hostid=fifo_id.hostid,path = fifo_id.path)
        except:
            ret["errors"] = "fifoId or fifo not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)  


        try:
            subject_id = SubId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"])
            subject = Subject.objects.get(sub_id=subject_id)

        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

# Subject label is getting changed

        if subject.admin in fifo.readers.all():
            subject.readers = list(set(fifo.readers.all()) & set(subject.readers.all()) )

            subject.writers = list( set(fifo.writers.all()) | set(subject.writers.all()) )
            subject.save()

        else:
            ret["status"] = 0
            ret["errors"] = "subject missing from pipe's reader list"
            return Response(ret, status=status.HTTP_404_NOT_FOUND)



        return Response(ret, status=status.HTTP_200_OK)






@api_view(['POST'])
def write_fifo(request):
    """
    write fifo on behalf of a subject.

    Example post parameters -

        {
        "sub_id": { "hostid": "8323329",  "uid": "1003", "pid": 111 }, 
        "fd" : 123
        }

    """
    if request.method == 'POST':

        ret = {}
        ret["status"] = 1
        mydata = request.data.copy()
        if(type(mydata) == dict):
            sub_id = mydata["sub_id"]
            fd = mydata["fd"]
        else:

            sub_id = ast.literal_eval(mydata.get("sub_id"))
            fd = ast.literal_eval(mydata.get("fd"))



        try:
            fifo_id = FifoId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"],fd=fd )
            #print "fifo_id",pipe_id.pipe_ref_num
            fifo = Fifo.objects.get(hostid=fifo_id.hostid,path = fifo_id.path)
        except:
            ret["errors"] = "fifoId or fifo not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)  


        try:
            subject_id = SubId.objects.get(hostid=sub_id["hostid"], uid=sub_id["uid"], pid=sub_id["pid"])
            subject = Subject.objects.get(sub_id=subject_id)

        except:
            ret["errors"] = "Subject not found."
            return Response(ret, status=status.HTTP_404_NOT_FOUND)

# Subject label is getting changed

        if subject.admin in fifo.writers.all():
            subject.readers = list(set(fifo.readers.all()) & set(subject.readers.all()) )

            subject.writers = list( set(fifo.writers.all()) | set(subject.writers.all()) )
            subject.save()

        else:
            ret["status"] = 0
            ret["errors"] = "subject missing from pipe's reader list"
            return Response(ret, status=status.HTTP_404_NOT_FOUND)



        return Response(ret, status=status.HTTP_200_OK)
