 docker run --rm  \
--cap-add=NET_RAW \
--cap-add=NET_ADMIN  \
--name="rawICPMJNI" \
--security-opt=no-new-privileges rawicmpjni_container
