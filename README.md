Simple dwm_status with snippets obtained from various sources.

I use it with the following in my .xinitrc:

while true; do
    xsetroot -name "$( dwm_status.py )"
    sleep 10
done &

