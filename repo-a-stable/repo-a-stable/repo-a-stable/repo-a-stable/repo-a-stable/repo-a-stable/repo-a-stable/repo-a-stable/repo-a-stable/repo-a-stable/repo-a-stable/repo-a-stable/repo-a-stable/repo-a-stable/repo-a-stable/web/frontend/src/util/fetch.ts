const fetchUrl = import.meta.env.VITE_API_BASE_URL || '';

export async function makeFetch(
    path: string,
    init: RequestInit = { credentials: 'include' }
): Promise<Response> {
    return fetch(`${fetchUrl}${path}`, init);
}
